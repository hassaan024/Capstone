import { Injectable, UnauthorizedException } from '@nestjs/common';
import { OAuth2Client } from 'google-auth-library';
import { PrismaService } from '../prisma/prisma.service';

@Injectable()
export class AuthService {
  private client = new OAuth2Client(
    process.env.GOOGLE_CLIENT_ID, // Use environment variable
  );

  constructor(private prisma: PrismaService) {}

  async verifyGoogleToken(token: string) {
    try {
      const ticket = await this.client.verifyIdToken({
        idToken: token,
        audience: process.env.GOOGLE_CLIENT_ID,
      });
      const payload = ticket.getPayload();
      
      if (!payload) throw new Error('Invalid payload');

      // upsert user
      const user = await this.prisma.user.upsert({
        where: { email: payload.email },
        update: {},
        create: {
            email: payload.email!,
            firstName: payload.given_name,
            lastName: payload.family_name,
            provider: 'google',
        },
      });

      return {
        user,
        message: 'Google login successful',
      };
    } catch (error) {
        console.error(error);
      throw new UnauthorizedException('Invalid Google Token');
    }
  }

  async register(data: { email: string; firstName: string; lastName: string; password?: string }) {
    const existing = await this.prisma.user.findUnique({ where: { email: data.email } });
    if (existing) {
        throw new UnauthorizedException('User already exists');
    }

    // In production, hash the password using bcrypt!
    // For this prototype, we store as plain text or assume simple handling
    const user = await this.prisma.user.create({
        data: {
            email: data.email,
            password: data.password, // TODO: Hash this
            firstName: data.firstName,
            lastName: data.lastName,
            provider: 'local',
        }
    });

    return { user, message: 'Registration successful' };
  }

  async login(data: { email: string; password: string }) {
      const user = await this.prisma.user.findUnique({ where: { email: data.email } });
      if (!user || user.password !== data.password) { // TODO: Compare hash
          throw new UnauthorizedException('Invalid credentials'); 
      }
      const { password, ...result } = user;
      return { user: result, message: 'Login successful' };
  }
}
