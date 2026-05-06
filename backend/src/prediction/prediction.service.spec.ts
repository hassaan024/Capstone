import { Test, TestingModule } from '@nestjs/testing';
import { PredictionService } from './prediction.service';
import { DatabaseService } from 'database/database.service';
import { WeatherService } from 'weather/weather.service';

const mockDb = {
  plantInstance: { findUnique: jest.fn(), update: jest.fn() },
  garden: { findUnique: jest.fn() },
  plantHistory: { deleteMany: jest.fn(), create: jest.fn() },
  species: { findFirst: jest.fn(), findUnique: jest.fn(), update: jest.fn(), create: jest.fn() },
};

const mockWeather = {
  getWeatherForGameDate: jest.fn(),
};

describe('PredictionService', () => {
  let service: PredictionService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        PredictionService,
        { provide: DatabaseService, useValue: mockDb },
        { provide: WeatherService, useValue: mockWeather },
      ],
    }).compile();

    service = module.get<PredictionService>(PredictionService);
    jest.clearAllMocks();
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });

  describe('bloomEstimate', () => {
    const baseSpecies = {
      id: 1,
      commonName: 'Test Plant',
      scientificName: 'Testus plantus',
      growthRate: 3,
      minHeight: 1,
      maxHeight: 2,
      avgHoursSun: 8,
      minTemp: 50,
      maxTemp: 100,
      wateringMinDays: 5,
      wateringMaxDays: 7,
      droughtTolerant: false,
      tropical: false,
      cycle: 'Annual',
      type: 'Flower',
      careLevel: 'Easy',
      family: null, genus: null, speciesEpithet: null, origin: null,
      flowers: null, floweringSeason: null, fruits: null, edibleFruit: null,
      notes: null, perenualId: null, trefleId: null, maintenance: null,
      wateringFreq: null, otherNames: null, harvestSeason: null,
      leaf: null, edibleLeaf: null, cuisine: null, medicinal: null,
      saltTolerant: null, indoor: null, thorny: null, invasive: null,
      pruningMonths: null, pruningFrequency: null, pruningInterval: null,
      plantAnatomy: null, imgSrcUrls: null,
      creationTimestamp: new Date(), lastUpdated: new Date(),
    };

    it('returns an estimated bloom date in the future', async () => {
      mockDb.species.findUnique.mockResolvedValue(baseSpecies);
      const planted = new Date('2026-01-01');
      const result = await service.bloomEstimate(1, planted);

      expect(result.estimatedBloomDate).toBeDefined();
      expect(new Date(result.estimatedBloomDate) > planted).toBe(true);
    });

    it('marks feasible true when species can reach full height within 730 days', async () => {
      // growthRate=3, maxHeight=2ft (60cm) — fast small plant, well under 730 days
      mockDb.species.findUnique.mockResolvedValue(baseSpecies);
      const result = await service.bloomEstimate(1, new Date('2026-01-01'));
      expect(result.feasible).toBe(true);
      expect(result.feasibilityNote).toBeNull();
    });

    it('marks feasible false for a slow enormous species', async () => {
      mockDb.species.findUnique.mockResolvedValue({
        ...baseSpecies,
        growthRate: 1,
        maxHeight: 300, // 300 feet — huge slow tree
      });
      const result = await service.bloomEstimate(1, new Date('2026-01-01'));
      expect(result.feasible).toBe(false);
      expect(result.feasibilityNote).toContain('days to reach full height');
    });

    it('throws NotFoundException when species does not exist', async () => {
      mockDb.species.findUnique.mockResolvedValue(null);
      await expect(service.bloomEstimate(999, new Date())).rejects.toThrow('Species 999 not found');
    });
  });

  describe('seedDemoSpecies', () => {
    it('creates species when none exist', async () => {
      mockDb.species.findFirst.mockResolvedValue(null);
      mockDb.species.create.mockResolvedValue({ id: 1 });

      const result = await service.seedDemoSpecies();

      expect(result.species).toHaveLength(6);
      expect(mockDb.species.create).toHaveBeenCalledTimes(6);
      expect(mockDb.species.update).not.toHaveBeenCalled();
    });

    it('updates species when they already exist', async () => {
      mockDb.species.findFirst.mockResolvedValue({ id: 99 });
      mockDb.species.update.mockResolvedValue({ id: 99 });

      const result = await service.seedDemoSpecies();

      expect(result.species).toHaveLength(6);
      result.species.forEach((s) => expect(s.action).toBe('updated'));
      expect(mockDb.species.create).not.toHaveBeenCalled();
    });

    it('returns all four demo species types', async () => {
      mockDb.species.findFirst.mockResolvedValue(null);
      mockDb.species.create.mockImplementation((args) =>
        Promise.resolve({ id: Math.random(), ...args.data }),
      );

      const result = await service.seedDemoSpecies();
      const types = result.species.map((s) => s.type);

      expect(types).toContain('Flower');
      expect(types).toContain('Vegetable');
      expect(types).toContain('Tree');
      expect(types).toContain('Shrub');
    });
  });
});
