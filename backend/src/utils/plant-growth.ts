import { GrowthStage } from 'enums/table_enums';
import { getDemoSpecies } from './demo-species';

export const TIMELINE_INTERVAL_DAYS = 7;

const MAX_DAYS_TO_MATURE = 730;

// Average real-world stress factor applied when estimating planting date.
// Perfect conditions never occur, so we scale down base daily growth.
// Expected days to reach mature display height at average stress (0.75),
export const AVG_STRESS_FACTOR = 0.9;

// indexed by [modelCategory][cycleType][growthRateTier 0=Low 1=Med 2=High].
// Reference heights: flower=60cm, vegetable=90cm, tree=300cm.
// Height scaling uses a mild ^0.3 exponent so unusually tall/short plants
// adjust proportionally without swinging wildly.
const EXPECTED_DAYS: Record<
  string,
  Record<string, [number, number, number]>
> = {
  flower: {
    annual: [105, 80, 65], // anchored: Zinnia (High)=60, marigold/petunia (Med)=70-90
    biennial: [600, 450, 300],
    perennial: [500, 210, 120], // anchored: Louisiana Iris (Med)=210
  },
  vegetable: {
    annual: [120, 80, 55], // anchored: Creole Tomato (Med)=80, Okra (High)=55
    perennial: [270, 180, 110],
  },
  tree: {
    annual: [130, 95, 65],
    perennial: [730, 730, 600],
  },
};

const REFERENCE_HEIGHT_CM: Record<string, number> = {
  flower: 60,
  vegetable: 90,
  tree: 300,
};

function cycleKey(
  cycle: string | null | undefined,
): 'annual' | 'biennial' | 'perennial' {
  const c = (cycle || '').toLowerCase();
  if (c.includes('annual') && !c.includes('biennial')) return 'annual';
  if (c.includes('biennial')) return 'biennial';
  return 'perennial';
}

/**
 * Raw (uncapped) estimate of days to reach maxHeightCm at average conditions.
 * This is the value shown to users and used for timeline length before capping.
 */
export function rawDaysToMature(
  growthRate: number,
  maxHeightCm: number,
  modelCategory: string | null | undefined = 'flower',
  cycle: string | null | undefined = '',
): number {
  const cat = (modelCategory || 'flower').toLowerCase();
  const ck = cycleKey(cycle);

  const tiers =
    EXPECTED_DAYS[cat]?.[ck] ?? EXPECTED_DAYS['flower']['perennial'];
  const idx = Math.max(0, Math.min(2, Math.round(growthRate) - 1));
  const baseDays = tiers[idx];

  const refHeight = REFERENCE_HEIGHT_CM[cat] ?? 60;
  const heightAdj = Math.pow(Math.max(maxHeightCm, 1) / refHeight, 0.3);

  return Math.ceil(baseDays * heightAdj);
}

/**
 * Capped estimate — same as rawDaysToMature but never exceeds MAX_DAYS_TO_MATURE.
 */
export function estimateDaysToMature(
  growthRate: number,
  maxHeightCm: number,
  modelCategory: string | null | undefined = 'flower',
  cycle: string | null | undefined = '',
): number {
  return Math.min(
    rawDaysToMature(growthRate, maxHeightCm, modelCategory, cycle),
    MAX_DAYS_TO_MATURE,
  );
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
  modelCategory?: string | null;
  cycle?: string | null;
}): number {
  const demo = getDemoSpecies(species.commonName, species.scientificName);
  if (demo) return demo.daysToFirstBloom;
  const maxHeightCm = (species.maxHeight ?? 1) * 30.48;
  return estimateDaysToMature(
    species.growthRate,
    maxHeightCm,
    species.modelCategory,
    species.cycle,
  );
}

/**
 * Maps a height-to-maxHeight ratio to a GrowthStage enum value.
 */
export function growthStageFromRatio(heightRatio: number): GrowthStage {
  if (heightRatio < 0.15) return GrowthStage.Seedling;
  if (heightRatio < 0.5) return GrowthStage.Vegetative;
  if (heightRatio < 0.85) return GrowthStage.Flowering;
  return GrowthStage.Fruiting;
}
