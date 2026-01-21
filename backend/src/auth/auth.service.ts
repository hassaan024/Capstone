import {
  Injectable,
  UnauthorizedException,
  BadRequestException,
  Logger,
} from '@nestjs/common';
import { UserService } from '../user/user.service.js';
import { RegisterDto } from './dto/register.dto.js';
import { LoginDto } from './dto/login.dto.js';
import * as bcrypt from 'bcrypt';

@Injectable()
export class AuthService {
  private readonly logger = new Logger(AuthService.name);

  constructor(private userService: UserService) {}

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
      
      this.logger.log(`✅ User registered successfully - ID: ${user.id}, Email: ${email}, Name: ${displayName}`);
      
      return {
        message: 'User registered successfully',
        user: userWithoutPassword,
      };
    } catch (error) {
      this.logger.error(`❌ Registration failed for ${email}: ${(error as Error).message}`);
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
    
    this.logger.log(`User logged in successfully - ID: ${user.id}, Email: ${email}, Name: ${user.displayName}`);
    
    return {
      message: 'Login successful',
      user: userWithoutPassword,
    };
  }

  async changePassword(userId: number, oldPassword: string, newPassword: string) {
    this.logger.log(`Password change attempt for user ID: ${userId}`);

    // Find user
    const user = await this.userService.findOne(userId);

    if (!user) {
      this.logger.warn(`Password change failed - User not found: ${userId}`);
      throw new UnauthorizedException('User not found');
    }

    // Verify old password
    if (!user.passwordHash) {
      this.logger.warn(`Password change failed - No password hash for user ID: ${userId}`);
      throw new UnauthorizedException('Invalid password');
    }

    const isPasswordValid = await bcrypt.compare(oldPassword, user.passwordHash);

    if (!isPasswordValid) {
      this.logger.warn(`Password change failed - Current password incorrect for user: ${user.email}`);
      throw new UnauthorizedException('Current password is incorrect');
    }

    // Hash new password
    const saltRounds = 10;
    const passwordHash = await bcrypt.hash(newPassword, saltRounds);

    // Update password
    await this.userService.update(userId, { passwordHash });

    this.logger.log(`Password changed successfully for user: ${user.email} (ID: ${userId})`);

    return {
      message: 'Password changed successfully',
    };
  }
}
