import { GrowthStage } from 'enums/table_enums';
import { getDemoSpecies } from './demo-species';

// Matches the tier lookup in the ML model (PREDICTION_MODEL.md §1)
const GROWTH_RATE_TIERS: Record<number, number> = { 1: 0.10, 2: 0.30, 3: 0.65 };

export const TIMELINE_INTERVAL_DAYS = 7;

// Cap growth timelines at 2 years to avoid unreasonably large weather fetches
const MAX_DAYS_TO_MATURE = 730;

// Average real-world stress factor applied when estimating planting date.
// Perfect conditions never occur, so we scale down base daily growth.
const AVG_STRESS_FACTOR = 0.6;

/**
 * Raw (uncapped) estimate of days to reach maxHeightCm.
 * Use this when you need to know the true growth duration for feasibility
 * checks — e.g. a large slow tree might need 2000+ days.
 */
export function rawDaysToMature(growthRate: number, maxHeightCm: number): number {
  const tier = GROWTH_RATE_TIERS[Math.round(growthRate)] ?? 0.30;
  const heightScale = Math.pow(maxHeightCm / 100, 0.4);
  const baseDailyGrowth = tier * heightScale;
  const realisticDailyGrowth = baseDailyGrowth * AVG_STRESS_FACTOR;
  return Math.ceil(maxHeightCm / realisticDailyGrowth);
}

/**
 * Capped estimate — same as rawDaysToMature but never exceeds MAX_DAYS_TO_MATURE.
 * Use this when calculating planted dates and timeline lengths.
 */
export function estimateDaysToMature(
  growthRate: number,
  maxHeightCm: number,
): number {
  return Math.min(rawDaysToMature(growthRate, maxHeightCm), MAX_DAYS_TO_MATURE);
}

/**
 * Computes bloom days for a species: uses hardcoded demo data when available,
 * otherwise falls back to the formula-based estimate.
 */
export function computeBloomDays(species: {
  commonName: string;
  scientificName: string;
  growthRate: number;
  maxHeight: number | null;
}): number {
  const demo = getDemoSpecies(species.commonName, species.scientificName);
  if (demo) return demo.daysToFirstBloom;
  const maxHeightCm = (species.maxHeight ?? 1) * 30.48;
  return estimateDaysToMature(species.growthRate, maxHeightCm);
}

/**
 * Maps a height-to-maxHeight ratio to a GrowthStage enum value.
 * Thresholds are chosen to give each stage a meaningful share of the
 * growth timeline while matching typical plant biology.
 */
export function growthStageFromRatio(heightRatio: number): GrowthStage {
  if (heightRatio < 0.15) return GrowthStage.Seedling;
  if (heightRatio < 0.50) return GrowthStage.Vegetative;
  if (heightRatio < 0.85) return GrowthStage.Flowering;
  return GrowthStage.Fruiting;
}
