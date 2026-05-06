// src/garden/garden.scheduler.ts
import { Injectable, Logger } from '@nestjs/common';
import { Cron, CronExpression } from '@nestjs/schedule';
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
   * Runs every day at 8:00 AM.
   * Finds all plant instances whose computed planting date falls EXACTLY 7 days
   * from today, then sends one email per affected user listing all such plants.
   *
   * Using "exactly 7 days away" avoids sending the same email on consecutive
   * days without needing a DB flag — each alert fires only once.
   */
  @Cron(CronExpression.EVERY_DAY_AT_8AM)
  async sendDailyPlantingAlerts() {
    this.logger.log('Running daily planting alert job…');

    const today = new Date();
    today.setHours(0, 0, 0, 0);
    // Target window: exactly 7 days from today (midnight → midnight)
    const windowStart = new Date(today.getTime() + 7 * 24 * 60 * 60 * 1000);
    const windowEnd   = new Date(windowStart.getTime() + 24 * 60 * 60 * 1000 - 1);

    // Fetch all gardens with plants and their owners
    const gardens = await this.db.garden.findMany({
      include: {
        plants: { include: { species: true } },
        owner:  true,
      },
    });

    // Group alerts by userId → { user, alerts[] }
    const byUser = new Map<
      number,
      { email: string; displayName: string; alerts: PlantAlertItem[] }
    >();

    for (const garden of gardens) {
      for (const plant of garden.plants) {
        // Compute planting date (same logic as getPlantingAlerts)
        let plantedDate: Date | null = null;

        if (plant.plantedDate) {
          plantedDate = new Date(plant.plantedDate);
        } else if (garden.bloomDate && plant.species.bloomDays) {
          const bloomMs = new Date(garden.bloomDate).getTime();
          plantedDate = new Date(
            bloomMs - plant.species.bloomDays * 24 * 60 * 60 * 1000,
          );
        } else if (garden.bloomDate) {
          // Fall back to computeBloomDays if species.bloomDays not set
          const bloomDays = computeBloomDays(plant.species as any);
          if (bloomDays) {
            const bloomMs = new Date(garden.bloomDate).getTime();
            plantedDate = new Date(bloomMs - bloomDays * 24 * 60 * 60 * 1000);
          }
        }

        if (!plantedDate) continue;

        // Normalise to midnight
        const pMidnight = new Date(plantedDate);
        pMidnight.setHours(0, 0, 0, 0);

        // Only alert if planting date is exactly 7 days away
        if (pMidnight < windowStart || pMidnight > windowEnd) continue;

        const owner = garden.owner as any;
        if (!owner?.email) continue;

        if (!byUser.has(owner.id)) {
          byUser.set(owner.id, {
            email: owner.email,
            displayName: owner.displayName ?? owner.email,
            alerts: [],
          });
        }

        byUser.get(owner.id)!.alerts.push({
          commonName:     plant.species.commonName ?? 'Unknown Plant',
          scientificName: plant.species.scientificName ?? '',
          plantedDate:    pMidnight.toISOString(),
          gardenName:     garden.name,
        });
      }
    }

    if (byUser.size === 0) {
      this.logger.log('No planting alerts due in exactly 7 days — no emails sent.');
      return;
    }

    // Send one email per user
    let sent = 0;
    for (const [, { email, displayName, alerts }] of byUser) {
      try {
        await this.emailService.sendPlantingAlert(email, displayName, alerts);
        sent++;
      } catch (err) {
        this.logger.error(`Failed to send alert email to ${email}: ${(err as Error).message}`);
      }
    }

    this.logger.log(`Daily alert job complete — ${sent}/${byUser.size} emails sent.`);
  }

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
   * Automatically called after a garden's bloomDate is updated OR after a new
   * PlantInstance is created. Checks if any plants in that garden now fall within
   * the 7-day planting window and, if so, immediately emails the garden's owner.
   *
   * Fire-and-forget — errors are logged but never rethrown so they never break
   * the parent create/update operation.
   */
  async triggerAlertEmailIfNeeded(gardenId: number): Promise<void> {
    try {
      const today = new Date();
      today.setHours(0, 0, 0, 0);
      const sevenDaysFromNow = new Date(today.getTime() + 7 * 24 * 60 * 60 * 1000);

      const garden = await this.db.garden.findUnique({
        where: { id: gardenId },
        include: {
          plants: { include: { species: true } },
          owner: true,
        },
      });

      if (!garden) return;

      const owner = garden.owner as any;
      if (!owner?.email) return;

      const alertItems = buildAlertItems(garden, today, sevenDaysFromNow);
      if (alertItems.length === 0) return;

      alertItems.sort((a, b) => new Date(a.plantedDate).getTime() - new Date(b.plantedDate).getTime());

      await this.emailService.sendPlantingAlert(
        owner.email,
        owner.displayName ?? owner.email,
        alertItems,
      );

      this.logger.log(
        `Auto-triggered alert email sent to ${owner.email} for garden "${garden.name}" (${alertItems.length} alerts)`,
      );
    } catch (err) {
      // Never let email failure break the save/create operation
      this.logger.error(
        `triggerAlertEmailIfNeeded failed for garden ${gardenId}: ${(err as Error).message}`,
      );
    }
  }
}
