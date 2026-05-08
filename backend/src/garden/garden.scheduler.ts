// src/garden/garden.scheduler.ts
import { Injectable, Logger } from '@nestjs/common';
import { DatabaseService } from 'database/database.service';
import { EmailService, PlantAlertItem } from '../email/email.service';
import { computeBloomDays } from 'utils/plant-growth';

/** Shared helper: build PlantAlertItem list from a garden's plant instances */
function buildAlertItems(
  garden: any,
  today: Date,
  sevenDaysFromNow: Date,
): PlantAlertItem[] {
  const items: PlantAlertItem[] = [];

  for (const plant of garden.plants ?? []) {
    let plantedDate: Date | null = null;

    if (plant.plantedDate) {
      plantedDate = new Date(plant.plantedDate);
    } else if (plant.bloomDate && plant.species?.bloomDays) {
      const bloomMs = new Date(plant.bloomDate).getTime();
      plantedDate = new Date(bloomMs - plant.species.bloomDays * 24 * 60 * 60 * 1000);
    } else if (plant.bloomDate) {
      const bloomDays = computeBloomDays(plant.species as any);
      if (bloomDays) {
        const bloomMs = new Date(plant.bloomDate).getTime();
        plantedDate = new Date(bloomMs - bloomDays * 24 * 60 * 60 * 1000);
      }
    } else if (garden.bloomDate && plant.species?.bloomDays) {
      const bloomMs = new Date(garden.bloomDate).getTime();
      plantedDate = new Date(bloomMs - plant.species.bloomDays * 24 * 60 * 60 * 1000);
    } else if (garden.bloomDate) {
      const bloomDays = computeBloomDays(plant.species as any);
      if (bloomDays) {
        const bloomMs = new Date(garden.bloomDate).getTime();
        plantedDate = new Date(bloomMs - bloomDays * 24 * 60 * 60 * 1000);
      }
    }

    if (!plantedDate) continue;

    const pMidnight = new Date(plantedDate);
    pMidnight.setHours(0, 0, 0, 0);

    if (pMidnight >= today && pMidnight <= sevenDaysFromNow) {
      items.push({
        commonName:     plant.species?.commonName ?? 'Unknown Plant',
        scientificName: plant.species?.scientificName ?? '',
        plantedDate:    pMidnight.toISOString(),
        gardenName:     garden.name,
      });
    }
  }

  return items;
}


@Injectable()
export class GardenScheduler {
  private readonly logger = new Logger(GardenScheduler.name);

  constructor(
    private readonly db: DatabaseService,
    private readonly emailService: EmailService,
  ) {}

  /**
   * On-demand version — sends alerts for ALL plants within the next 7 days
   * for a single user. Used by the frontend "Email me now" button.
   */
  async sendAlertEmailForUser(userId: number): Promise<{ sent: boolean; alertCount: number }> {
    const user = await this.db.user.findUnique({ where: { id: userId } });
    if (!user?.email) return { sent: false, alertCount: 0 };

    const today = new Date();
    today.setHours(0, 0, 0, 0);
    const sevenDaysFromNow = new Date(today.getTime() + 7 * 24 * 60 * 60 * 1000);

    const gardens = await this.db.garden.findMany({
      where: { ownerId: userId },
      include: { plants: { include: { species: true } } },
    });

    const alertItems: PlantAlertItem[] = gardens.flatMap(g =>
      buildAlertItems(g, today, sevenDaysFromNow),
    );

    if (alertItems.length === 0) return { sent: false, alertCount: 0 };

    alertItems.sort((a, b) => new Date(a.plantedDate).getTime() - new Date(b.plantedDate).getTime());

    await this.emailService.sendPlantingAlert(
      user.email,
      user.displayName ?? user.email,
      alertItems,
    );

    this.logger.log(`On-demand alert email sent to ${user.email} (${alertItems.length} alerts)`);
    return { sent: true, alertCount: alertItems.length };
  }

  /**
   * Schedules a planting alert email to be sent at a specific datetime.
   * Uses Node.js setTimeout — works perfectly for demo delays up to ~24h.
   * Returns the scheduled ISO time and delay in seconds so the frontend can confirm.
   */
  async scheduleAlertEmailForUser(
    userId: number,
    scheduledAt: string,
  ): Promise<{ scheduled: boolean; scheduledAt: string; delaySeconds: number }> {
    const user = await this.db.user.findUnique({ where: { id: userId } });
    if (!user?.email) return { scheduled: false, scheduledAt, delaySeconds: 0 };

    const targetTime = new Date(scheduledAt).getTime();
    const now = Date.now();
    const delayMs = Math.max(0, targetTime - now);
    const delaySeconds = Math.round(delayMs / 1000);

    this.logger.log(
      `Email for user ${userId} scheduled in ${delaySeconds}s (at ${new Date(scheduledAt).toLocaleTimeString()})`,
    );

    // Fire after the delay
    setTimeout(async () => {
      try {
        const result = await this.sendAlertEmailForUser(userId);
        if (result.sent) {
          this.logger.log(`Scheduled email sent to ${user.email} (${result.alertCount} alerts)`);
        } else {
          this.logger.warn(`Scheduled email: no alerts found for ${user.email} at send time`);
        }
      } catch (err) {
        this.logger.error(`Scheduled email failed for ${user.email}: ${(err as Error).message}`);
      }
    }, delayMs);

    return { scheduled: true, scheduledAt, delaySeconds };
  }
}
