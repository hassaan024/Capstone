import { estimateDaysToMature, rawDaysToMature, growthStageFromRatio } from './plant-growth';
import { GrowthStage } from 'enums/table_enums';

describe('estimateDaysToMature', () => {
  it('faster growth rate means fewer days', () => {
    const slow = estimateDaysToMature(1, 100);
    const medium = estimateDaysToMature(2, 100);
    const fast = estimateDaysToMature(3, 100);
    expect(slow).toBeGreaterThan(medium);
    expect(medium).toBeGreaterThan(fast);
  });

  it('taller plant takes longer even at the same growth rate', () => {
    const short = estimateDaysToMature(2, 50);
    const tall = estimateDaysToMature(2, 200);
    expect(tall).toBeGreaterThan(short);
  });

  it('caps at 730 days for very slow/large species', () => {
    const huge = estimateDaysToMature(1, 3000); // enormous slow tree
    expect(huge).toBe(730);
  });

  it('returns a positive integer', () => {
    const days = estimateDaysToMature(2, 150);
    expect(days).toBeGreaterThan(0);
    expect(Number.isInteger(days)).toBe(true);
  });
});

describe('rawDaysToMature', () => {
  it('returns more days than estimateDaysToMature for a species that needs over 730 days', () => {
    const raw = rawDaysToMature(1, 3000);
    const capped = estimateDaysToMature(1, 3000);
    expect(raw).toBeGreaterThan(730);
    expect(capped).toBe(730);
  });

  it('matches estimateDaysToMature when under the cap', () => {
    const raw = rawDaysToMature(3, 60);
    const capped = estimateDaysToMature(3, 60);
    expect(raw).toBe(capped);
    expect(raw).toBeLessThan(730);
  });
});

describe('growthStageFromRatio', () => {
  it('returns Seedling below 15%', () => {
    expect(growthStageFromRatio(0)).toBe(GrowthStage.Seedling);
    expect(growthStageFromRatio(0.14)).toBe(GrowthStage.Seedling);
  });

  it('returns Vegetative between 15% and 50%', () => {
    expect(growthStageFromRatio(0.15)).toBe(GrowthStage.Vegetative);
    expect(growthStageFromRatio(0.49)).toBe(GrowthStage.Vegetative);
  });

  it('returns Flowering between 50% and 85%', () => {
    expect(growthStageFromRatio(0.50)).toBe(GrowthStage.Flowering);
    expect(growthStageFromRatio(0.84)).toBe(GrowthStage.Flowering);
  });

  it('returns Fruiting at 85% and above', () => {
    expect(growthStageFromRatio(0.85)).toBe(GrowthStage.Fruiting);
    expect(growthStageFromRatio(1.0)).toBe(GrowthStage.Fruiting);
  });
});
