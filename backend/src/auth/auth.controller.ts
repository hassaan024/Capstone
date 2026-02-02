import {
  Controller,
  Get,
  Post,
  Body,
  HttpCode,
  HttpStatus,
  Param,
  Query,
  BadRequestException,
  Res,
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
import type { Response } from 'express';

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

  // @Get('google/:origin')
  // async handleGoogleOAuthRedirect(
  //   @Param('origin', OriginValidationPipe) origin: Origin,
  //   @Query('code') code?: string,
  //   @Query('state') sid?: string,
  // ): Promise<UpdateUserDto> {
  //   console.log('GET HTITING BACKEND');
  //   console.log(`SID: ${sid}`);
  //   if (!code) {
  //     throw new BadRequestException('Missing Google auth code');
  //   }

  //   return this.authService.handleGoogleOAuth(origin, code, sid);
  // }

  @Get('google/:origin')
  async handleGoogleOAuthRedirect(
    @Res() res: Response,
    @Param('origin', OriginValidationPipe) origin: Origin,
    @Query('code') code?: string,
    @Query('state') sid?: string,
  ) {
    if (!code) throw new BadRequestException('Missing Google auth code');
    if (origin === 'unreal' && !sid) throw new BadRequestException('Missing state');

    try {
      // Your service should do: exchange code, fetch userinfo, upsert, AND (for unreal) push to UE using sid.
      const ok = await this.authService.handleGoogleOAuth(origin, code, sid);

      return res
        .status(200)
        .type('html')
        .send(this.renderLoginDonePage(true));
    } catch (e) {
      return res
        .status(200)
        .type('html')
        .send(this.renderLoginDonePage(false));
    }
  }

  private renderLoginDonePage(ok: boolean) {
    const title = ok ? 'Login complete' : 'Login failed';
    const msg = ok
      ? 'Login is complete. You may close this tab and return to the app or visit the website.'
      : 'Login failed. Please return to the app and try again.';

    return `<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>${title}</title>
  <style>
    body { margin:0; font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; background:#0b0f19; color:#e6e8ef; }
    .wrap { min-height: 100vh; display:flex; align-items:center; justify-content:center; padding:24px; }
    .card { max-width:560px; width:100%; background:#121a2b; border:1px solid #22304d; border-radius:16px; padding:24px; box-shadow: 0 10px 30px rgba(0,0,0,.35); }
    h1 { font-size:20px; margin:0 0 10px; }
    p { margin:0 0 16px; line-height:1.45; color:#b9c0d4; }
    button { cursor:pointer; background:#2a5bd7; color:white; border:0; border-radius:10px; padding:10px 14px; font-weight:600; }
    .small { font-size:12px; color:#8d96ad; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>${title}</h1>
      <p>${msg}</p>
    </div>
  </div>
</body>
</html>`;
  }


  @Post('google/:origin')
  @HttpCode(HttpStatus.OK)
  async handleGoogleOAuth(
    @Param('origin', OriginValidationPipe) origin: Origin,
    @Body() googleOAuthDto: GoogleOAuthDto,
    @Query('code') codeQuery?: string, // optional query param used for unreal
  ): Promise<UpdateUserDto> {
    console.log('HITTING BACKEND')
    // Use the query param if origin is unreal, otherwise use body
    const code = origin === 'unreal' ? codeQuery : googleOAuthDto.code;

    if (!code) {
      throw new BadRequestException('Missing Google auth code');
    }

    return await this.authService.handleGoogleOAuth(
      origin, 
      code);

  }
}
