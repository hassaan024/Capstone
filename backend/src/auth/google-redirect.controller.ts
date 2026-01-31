import { Controller, Get, Query, Res, BadRequestException } from '@nestjs/common';
import type { Response } from 'express';
import { AuthService } from './auth.service.js';
import { GOOGLE_CLIENTS } from '../utils/constants.js';
import { Logger } from '@nestjs/common';

type SidEntry = { returnPort: number; createdAt: number };
const SID_TTL_MS = 2 * 60 * 1000;
const sidStore = new Map<string, SidEntry>();

function putSid(sid: string, returnPort: number) {
  sidStore.set(sid, { returnPort, createdAt: Date.now() });
}

function popSid(sid: string): SidEntry | null {
  const e = sidStore.get(sid);
  sidStore.delete(sid);
  if (!e) return null;
  if (Date.now() - e.createdAt > SID_TTL_MS) return null;
  return e;
}

async function sleep(ms: number) {
  await new Promise((r) => setTimeout(r, ms));
}

@Controller('auth/google/unreal')
export class GoogleUnrealRedirectController {
  constructor(private readonly authService: AuthService) {}
  private readonly logger = new Logger(GoogleUnrealRedirectController.name);

  @Get('start')
  async start(
    @Query('sid') sid: string,
    @Query('return_port') returnPortStr: string,
    @Res() res: Response,
  ) {
    if (!sid) throw new BadRequestException('Missing sid');

    const returnPort = Number(returnPortStr);
    if (!Number.isFinite(returnPort) || returnPort <= 0 || returnPort > 65535) {
      throw new BadRequestException('Invalid return_port');
    }

    putSid(sid, returnPort);

    const client = GOOGLE_CLIENTS.UNREAL;
    if (!client?.AUTH_URI || !client?.CLIENT_ID || !client?.REDIRECT_URI) {
      throw new BadRequestException('UNREAL Google client env vars not set');
    }

    // IMPORTANT: redirect_uri must match EXACTLY what you later send to token exchange.
    const params = new URLSearchParams({
      client_id: client.CLIENT_ID,
      redirect_uri: client.REDIRECT_URI,
      response_type: 'code',
      scope: 'openid email profile',
      access_type: 'offline',
      prompt: 'select_account',
      state: sid,
    });

    const authUrl = `${client.AUTH_URI}?${params.toString()}`;
    return res.redirect(authUrl);
  }

  @Get('callback')
  async callback(
    @Query('code') code: string,
    @Query('state') sid: string,
    @Res() res: Response,
  ) {
    if (!code) throw new BadRequestException('Missing code');
    if (!sid) throw new BadRequestException('Missing state');

    const entry = popSid(sid);
    if (!entry) {
      return res
        .status(400)
        .send('<html><body><h3>Login session expired.</h3>Return to the app and try again.</body></html>');
    }

    try {
  const userDto = await this.authService.handleGoogleOAuth('unreal', code);

//   const pushUrl = `http://127.0.0.1:${entry.returnPort}/oauth/complete`;
  const pushUrl = `http://host.docker.internal:${entry.returnPort}/oauth/complete`;

//   const payload = { sid, user: userDto };
  const payload = {
    sid,
    session_token: JSON.stringify(userDto), // TEMP: until you add real sessions
  };

  this.logger.log(`[OAUTH] sid=${sid}`);
  this.logger.log(`[OAUTH] pushing to UE at ${pushUrl}`);
  this.logger.log(`[OAUTH] payload keys = ${Object.keys(payload).join(', ')}`);

  let pushed = false;

  for (const delay of [0, 150, 400]) {
    if (delay) await sleep(delay);

    try {
    const r = await fetch(pushUrl, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
    });

    const text = await r.text().catch(() => '');
    this.logger.log(`[OAUTH] UE push status=${r.status} ok=${r.ok} body="${text}"`);

    if (r.ok) { pushed = true; break; }
    } catch (e) {
    this.logger.error(`[OAUTH] UE push threw: ${(e as Error).message}`);
    }

  }

  if (!pushed) {
    return res
      .status(200)
      .send('<html><body><h3>Login completed, but the app did not receive it.</h3>Please return to the app and try again.</body></html>');
  }

  return res
    .status(200)
    .send('<html><body><h3>Login complete.</h3>You can close this tab and return to the app.</body></html>');
    } catch (e: any) {
        this.logger.error(`[OAUTH] UE push threw: ${e?.message}`);
        if (e?.cause) {
            this.logger.error(`[OAUTH] UE push cause: ${e.cause?.code ?? ''} ${e.cause?.message ?? e.cause}`);
        }
    }
  }
}
