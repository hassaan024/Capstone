import { DEMO_SPECIES, getDemoSpecies } from './demo-species';

describe('DEMO_SPECIES', () => {
  it('contains all four species types', () => {
    const types = DEMO_SPECIES.map((s) => s.type);
    expect(types).toContain('Flower');
    expect(types).toContain('Vegetable');
    expect(types).toContain('Tree');
    expect(types).toContain('Shrub');
    expect(DEMO_SPECIES).toHaveLength(6);
  });

  it('every species has all ML-critical fields populated', () => {
    for (const s of DEMO_SPECIES) {
      expect(s.growthRate).toBeGreaterThan(0);
      expect(s.maxHeight).toBeGreaterThan(0);
      expect(s.avgHoursSun).toBeGreaterThan(0);
      expect(s.minTemp).toBeDefined();
      expect(s.maxTemp).toBeGreaterThan(s.minTemp);
      expect(s.wateringMinDays).toBeGreaterThan(0);
      expect(s.wateringMaxDays).toBeGreaterThanOrEqual(s.wateringMinDays);
    }
  });

  it('every species has daysToFirstBloom and sowingInfo', () => {
    for (const s of DEMO_SPECIES) {
      expect(s.daysToFirstBloom).toBeGreaterThan(0);
      expect(s.sowingInfo).toBeTruthy();
    }
  });

  it('temperature ranges are valid for Louisiana', () => {
    for (const s of DEMO_SPECIES) {
      // All demo plants should tolerate at least 95°F Louisiana summer heat
      expect(s.maxTemp).toBeGreaterThanOrEqual(95);
    }
  });
});

describe('getDemoSpecies', () => {
  it('matches by commonName (case-insensitive)', () => {
    const result = getDemoSpecies('profusion zinnia', 'something');
    expect(result).not.toBeNull();
    expect(result!.type).toBe('Flower');
  });

  it('matches by scientificName (case-insensitive)', () => {
    const result = getDemoSpecies('unknown name', 'Solanum lycopersicum');
    expect(result).not.toBeNull();
    expect(result!.type).toBe('Vegetable');
  });

  it('returns null for unknown species', () => {
    expect(getDemoSpecies('Oak Tree', 'Quercus robur')).toBeNull();
  });

  it('matches all six demo species by name', () => {
    expect(getDemoSpecies('Profusion Zinnia', '')?.type).toBe('Flower');
    expect(getDemoSpecies('', 'Solanum lycopersicum')?.type).toBe('Vegetable');
    expect(getDemoSpecies('Crape Myrtle', '')?.type).toBe('Tree');
    expect(getDemoSpecies('Southern Azalea', '')?.type).toBe('Shrub');
    expect(getDemoSpecies('', 'Iris louisiana')?.type).toBe('Flower');
    expect(getDemoSpecies('Okra', '')?.type).toBe('Vegetable');
  });
});
