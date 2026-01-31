import {
  Controller,
  Post,
  Body,
  HttpCode,
  HttpStatus,
  Param,
  Query,
  BadRequestException,
} from '@nestjs/common';
import { AuthService } from './auth.service.js';
import { RegisterDto } from './dto/register.dto.js';
import { LoginDto } from './dto/login.dto.js';
import { ChangePasswordDto } from './dto/change-password.dto.js';
import { GoogleOAuthDto } from './dto/google-oauth.dto.js';
import { UpdateUserDto } from '../user/dto/update-user.dto.js';
import {
  Origin,
  OriginValidationPipe,
} from './pipes/origin-validation.pipe.js';

@Controller('auth')
export class AuthController {
  constructor(private readonly authService: AuthService) {}

  @Post('register')
  async register(@Body() registerDto: RegisterDto) {
    return this.authService.register(registerDto);
  }

  @Post('login')
  @HttpCode(HttpStatus.OK)
  async login(@Body() loginDto: LoginDto) {
    return this.authService.login(loginDto);
  }

  @Post('change-password')
  @HttpCode(HttpStatus.OK)
  async changePassword(@Body() changePasswordDto: ChangePasswordDto) {
    return this.authService.changePassword(
      changePasswordDto.userId,
      changePasswordDto.oldPassword,
      changePasswordDto.newPassword,
    );
  }

  @Post('google/:origin')
  @HttpCode(HttpStatus.OK)
  async handleGoogleOAuth(
    @Param('origin', OriginValidationPipe) origin: Origin,
    @Body() googleOAuthDto: GoogleOAuthDto,
    @Query('code') codeQuery?: string, // optional query param used for unreal
  ): Promise<UpdateUserDto> {
    // Use the query param if origin is unreal, otherwise use body
    const code = origin === 'unreal' ? codeQuery : googleOAuthDto.code;

    if (!code) {
      throw new BadRequestException('Missing Google auth code');
    }

    // return await this.authService.handleGoogleOAuth(
    //   origin,
    //   googleOAuthDto.code,
    // );

    return await this.authService.handleGoogleOAuth(
      origin, 
      code);

  }
}
