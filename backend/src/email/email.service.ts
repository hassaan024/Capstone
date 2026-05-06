// src/email/email.service.ts
import { Injectable, Logger } from '@nestjs/common';
import * as nodemailer from 'nodemailer';
import { ConfigService } from '@nestjs/config';

export interface PlantAlertItem {
  commonName: string;
  scientificName: string;
  plantedDate: string; // ISO string
  gardenName: string;
}

@Injectable()
export class EmailService {
  private readonly logger = new Logger(EmailService.name);
  private transporter: nodemailer.Transporter;

  constructor(private config: ConfigService) {
    this.transporter = nodemailer.createTransport({
      service: 'gmail',
      auth: {
        user: this.config.get<string>('EMAIL_USER'),
        pass: this.config.get<string>('EMAIL_APP_PASSWORD'),
      },
    });
  }

  async sendPlantingAlert(
    toEmail: string,
    displayName: string,
    alerts: PlantAlertItem[],
  ): Promise<void> {
    if (alerts.length === 0) return;

    const fromName =
      this.config.get<string>('EMAIL_FROM_NAME') ?? 'LeafyLedger';
    const fromEmail = this.config.get<string>('EMAIL_USER');

    const formatDate = (iso: string) =>
      new Date(iso).toLocaleDateString('en-US', {
        month: 'long',
        day: 'numeric',
        year: 'numeric',
        timeZone: 'UTC',
      });

    const rows = alerts
      .map(
        (a) => `
        <tr>
          <td style="padding:10px 14px;border-bottom:1px solid #1e293b;color:#e2e8f0;font-weight:600;">${a.commonName}</td>
          <td style="padding:10px 14px;border-bottom:1px solid #1e293b;color:#94a3b8;font-style:italic;">${a.scientificName}</td>
          <td style="padding:10px 14px;border-bottom:1px solid #1e293b;color:#fcd34d;font-weight:600;">${formatDate(a.plantedDate)}</td>
          <td style="padding:10px 14px;border-bottom:1px solid #1e293b;color:#86efac;">${a.gardenName}</td>
        </tr>`,
      )
      .join('');

    const html = `
<!DOCTYPE html>
<html>
<head><meta charset="utf-8"></head>
<body style="margin:0;padding:0;background:#0f172a;font-family:'Segoe UI',Arial,sans-serif;">
  <table width="100%" cellpadding="0" cellspacing="0" style="background:#0f172a;padding:32px 16px;">
    <tr><td align="center">
      <table width="600" cellpadding="0" cellspacing="0" style="background:linear-gradient(145deg,#1e293b,#0f172a);border:1px solid rgba(74,222,128,0.2);border-radius:16px;overflow:hidden;max-width:600px;width:100%;">

        <!-- Header -->
        <tr>
          <td style="background:linear-gradient(135deg,rgba(22,101,52,0.6),rgba(15,23,42,0.9));padding:32px 40px;border-bottom:1px solid rgba(74,222,128,0.15);">
            <p style="margin:0 0 6px;font-size:13px;text-transform:uppercase;letter-spacing:0.1em;color:rgba(134,239,172,0.7);">LeafyLedger</p>
            <h1 style="margin:0;font-size:24px;color:#f1f5f9;font-weight:700;">Planting Reminder</h1>
            <p style="margin:8px 0 0;font-size:15px;color:rgba(148,163,184,0.9);">Hi ${displayName}, you have plants to place this week.</p>
          </td>
        </tr>

        <!-- Body -->
        <tr>
          <td style="padding:32px 40px;">
            <p style="margin:0 0 20px;font-size:15px;color:rgba(203,213,225,0.85);line-height:1.6;">
              The following ${alerts.length === 1 ? 'plant needs' : `${alerts.length} plants need`} to be placed in the next <strong style="color:#86efac;">7 days</strong> to hit your target bloom dates:
            </p>

            <!-- Table -->
            <table width="100%" cellpadding="0" cellspacing="0" style="border-collapse:collapse;border:1px solid #1e293b;border-radius:10px;overflow:hidden;">
              <thead>
                <tr style="background:#0f172a;">
                  <th style="padding:10px 14px;text-align:left;font-size:11px;text-transform:uppercase;letter-spacing:0.07em;color:rgba(148,163,184,0.6);font-weight:600;">Plant</th>
                  <th style="padding:10px 14px;text-align:left;font-size:11px;text-transform:uppercase;letter-spacing:0.07em;color:rgba(148,163,184,0.6);font-weight:600;">Scientific Name</th>
                  <th style="padding:10px 14px;text-align:left;font-size:11px;text-transform:uppercase;letter-spacing:0.07em;color:rgba(148,163,184,0.6);font-weight:600;">Plant By</th>
                  <th style="padding:10px 14px;text-align:left;font-size:11px;text-transform:uppercase;letter-spacing:0.07em;color:rgba(148,163,184,0.6);font-weight:600;">Garden</th>
                </tr>
              </thead>
              <tbody style="background:#1e293b;">
                ${rows}
              </tbody>
            </table>

            <p style="margin:24px 0 0;font-size:14px;color:rgba(148,163,184,0.7);line-height:1.6;">
              Open the <strong style="color:#86efac;">LeafyLedger app</strong> to place your plants in the 3D garden view.
            </p>
          </td>
        </tr>

        <!-- Footer -->
        <tr>
          <td style="padding:20px 40px;border-top:1px solid rgba(148,163,184,0.1);text-align:center;">
            <p style="margin:0;font-size:12px;color:rgba(148,163,184,0.4);">
              You're receiving this because you have plants with upcoming planting dates.<br>
              LeafyLedger 🌿
            </p>
          </td>
        </tr>

      </table>
    </td></tr>
  </table>
</body>
</html>`;

    try {
      await this.transporter.sendMail({
        from: `"${fromName}" <${fromEmail}>`,
        to: toEmail,
        subject: `LeafyLedger — You have ${alerts.length} plant${alerts.length > 1 ? 's' : ''} to place this week`,
        html,
      });
      this.logger.log(
        `Planting alert email sent to ${toEmail} (${alerts.length} alerts)`,
      );
    } catch (err) {
      this.logger.error(
        `Failed to send planting alert email to ${toEmail}: ${(err as Error).message}`,
      );
      throw err;
    }
  }
}
