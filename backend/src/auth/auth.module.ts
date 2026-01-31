import { Module } from '@nestjs/common';
import { AuthService } from './auth.service.js';
import { AuthController } from './auth.controller.js';
import { GoogleUnrealRedirectController } from './google-redirect.controller.js';
import { UserModule } from '../user/user.module.js';

@Module({
  imports: [UserModule],
  controllers: [AuthController, GoogleUnrealRedirectController],
  providers: [AuthService],
})
export class AuthModule {}
