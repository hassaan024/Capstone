import {
  Injectable,
  UnauthorizedException,
  Logger,
  HttpStatus,
  HttpException,
} from '@nestjs/common';
import { UserService } from '../user/user.service.js';
import { RegisterDto } from './dto/register.dto.js';
import { LoginDto } from './dto/login.dto.js';
import * as bcrypt from 'bcrypt';
import { UpdateUserDto } from '../user/dto/update-user.dto.js';
import { Origin } from './pipes/origin-validation.pipe.js';
import { GOOGLE_CLIENTS } from '../utils/constants.js';
import { DatabaseService } from '../database/database.service.js';
import { sendGoogleAuthInfoToUnrealTCP } from '../utils/fetches.js';

interface GoogleTokenResponse {
  access_token: string;
  expires_in: number;
  refresh_token?: string;
  scope: string;
  token_type: string;
  id_token: string;
}

interface GoogleProfileInfo {
  id: string;
  email: string;
  verified_email: boolean;
  name: string;
  given_name: string;
  family_name: string;
  picture: string;
}

@Injectable()
export class AuthService {
  private readonly logger = new Logger(AuthService.name);

  constructor(
    private userService: UserService,
    private db: DatabaseService,
  ) {}

  async register(registerDto: RegisterDto) {
    const { email, password, firstName, lastName } = registerDto;

    this.logger.log(`Registration attempt for email: ${email}`);

    // Hash the password
    const saltRounds = 10;
    const passwordHash = await bcrypt.hash(password, saltRounds);

    // Create display name from first and last name
    const displayName = `${firstName} ${lastName}`;

    // Create user via UserService
    try {
      const user = await this.userService.create({
        email,
        displayName,
        passwordHash,
      });

      // Return user without password hash
      const { passwordHash: _, ...userWithoutPassword } = user;

      this.logger.log(
        `✅ User registered successfully - ID: ${user.id}, Email: ${email}, Name: ${displayName}`,
      );

      return {
        message: 'User registered successfully',
        user: userWithoutPassword,
      };
    } catch (error) {
      this.logger.error(
        `❌ Registration failed for ${email}: ${(error as Error).message}`,
      );
      // Re-throw the error from UserService (e.g., duplicate email)
      throw error;
    }
  }

  async login(loginDto: LoginDto) {
    const { email, password } = loginDto;

    this.logger.log(`Login attempt for email: ${email}`);

    // Find user by email
    const users = await this.userService.findAll();
    const user = users.find((u) => u.email === email);

    if (!user) {
      this.logger.warn(`❌ Login failed - User not found: ${email}`);
      throw new UnauthorizedException('Invalid email or password');
    }

    // Verify password
    if (!user.passwordHash) {
      this.logger.warn(`❌ Login failed - No password hash for user: ${email}`);
      throw new UnauthorizedException('Invalid email or password');
    }

    const isPasswordValid = await bcrypt.compare(password, user.passwordHash);

    if (!isPasswordValid) {
      this.logger.warn(`Login failed - Invalid password for: ${email}`);
      throw new UnauthorizedException('Invalid email or password');
    }

    // Return user without password hash
    const { passwordHash: _, ...userWithoutPassword } = user;

    this.logger.log(
      `User logged in successfully - ID: ${user.id}, Email: ${email}, Name: ${user.displayName}`,
    );

    return {
      message: 'Login successful',
      user: userWithoutPassword,
    };
  }

  async changePassword(
    userId: number,
    oldPassword: string,
    newPassword: string,
  ) {
    this.logger.log(`Password change attempt for user ID: ${userId}`);

    // Find user
    const user = await this.userService.findOne(userId);

    if (!user) {
      this.logger.warn(`Password change failed - User not found: ${userId}`);
      throw new UnauthorizedException('User not found');
    }

    // Verify old password
    if (!user.passwordHash) {
      this.logger.warn(
        `Password change failed - No password hash for user ID: ${userId}`,
      );
      throw new UnauthorizedException('Invalid password');
    }

    const isPasswordValid = await bcrypt.compare(
      oldPassword,
      user.passwordHash,
    );

    if (!isPasswordValid) {
      this.logger.warn(
        `Password change failed - Current password incorrect for user: ${user.email}`,
      );
      throw new UnauthorizedException('Current password is incorrect');
    }

    // Hash new password
    const saltRounds = 10;
    const passwordHash = await bcrypt.hash(newPassword, saltRounds);

    // Update password
    await this.userService.update(userId, { passwordHash });

    this.logger.log(
      `Password changed successfully for user: ${user.email} (ID: ${userId})`,
    );

    return {
      message: 'Password changed successfully',
    };
  }

async handleGoogleOAuth(
    origin: Origin,
    auth_code: string,
    sid: string | undefined = undefined,
  ): Promise<UpdateUserDto> {
    const client_info =
      origin === 'unreal' ? GOOGLE_CLIENTS.UNREAL : GOOGLE_CLIENTS.REACT;

    this.logger.log(`
      AUTH CODE: ${auth_code}
      ORIGIN: ${origin}
    `);

      this.logger.log(`
        redirect_uri: ${client_info.REDIRECT_URI}`)

    // auth code for access token
    const token_response = await fetch(client_info.TOKEN_URI, {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: new URLSearchParams({
        code: auth_code,
        client_id: client_info.CLIENT_ID,
        client_secret: client_info.CLIENT_SECRET,
        redirect_uri: client_info.REDIRECT_URI,
        grant_type: 'authorization_code',
      }).toString(),
    });

    if (!token_response.ok) {
      throw new HttpException(
        `Google token exchange failed: ${token_response.statusText}`,
        HttpStatus.BAD_REQUEST,
      );
    }
    console.log('WE GOT A RESPONSE');

    const tokens: GoogleTokenResponse =
      (await token_response.json()) as GoogleTokenResponse;

    this.logger.log('Tokens received:', tokens);

    // exchange token for user info
    const userInfoResponse = await fetch(
      'https://www.googleapis.com/oauth2/v2/userinfo',
      {
        headers: {
          Authorization: `Bearer ${tokens.access_token}`,
        },
      },
    );

    if (!userInfoResponse.ok) {
      const errorBody = await userInfoResponse.text();
      throw new HttpException(
        `Failed to fetch Google user info: ${userInfoResponse.status} - ${errorBody}`,
        HttpStatus.BAD_REQUEST,
      );
    }

    const userInfo: GoogleProfileInfo =
      (await userInfoResponse.json()) as GoogleProfileInfo;

    this.logger.log('Google user info:', userInfo);

    // insert into database or update
    const user = await this.db.user.upsert({
      where: { googleId: userInfo.id },
      update: {
        email: userInfo.email,
        verifiedEmail: userInfo.verified_email,
        googleDisplayName: userInfo.name,
        givenName: userInfo.given_name,
        familyName: userInfo.family_name,
        picture: userInfo.picture,
        lastUpdated: new Date(),
      },
      create: {
        email: userInfo.email,
        displayName: userInfo.name,
        googleId: userInfo.id,
        verifiedEmail: userInfo.verified_email,
        googleDisplayName: userInfo.name,
        givenName: userInfo.given_name,
        familyName: userInfo.family_name,
        picture: userInfo.picture,
      },
    });

    // Map to result object excluding passwordHash but including DB stamps
    const { passwordHash: _, ...userWithoutPassword } = user;
    
    // Any is used here to bridge strictly generated types with dynamic auth objects
    const result: any = {
      ...userWithoutPassword,
      sid: sid,
    };

    if (origin === 'unreal') {
      this.logger.log('trying to send info to unreal');
      await sendGoogleAuthInfoToUnrealTCP(result);
    }

    this.logger.log(`Auth result constructed for ${user.email}`);

    return result;
  }
}
