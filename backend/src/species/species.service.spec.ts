import { Test, TestingModule } from '@nestjs/testing';
import { NotFoundException } from '@nestjs/common';
import { SpeciesService } from './species.service';
import { DatabaseService } from 'database/database.service';
import { PerenualService } from 'perenual/perenual.service';

const mockSpecies = {
  id: 1,
  commonName: 'Profusion Zinnia',
  scientificName: 'Zinnia elegans',
  growthRate: 3,
  maxHeight: 2.0,
  bloomDays: null,
  type: 'Flower',
  edibleFruit: false,
  edibleLeaf: false,
  perenualId: 100,
  trefleId: null,
};

const mockDb = {
  user: {
    findUnique: jest.fn(),
    update: jest.fn(),
  },
  species: {
    findUnique: jest.fn(),
    update: jest.fn(),
    create: jest.fn(),
    findMany: jest.fn(),
    delete: jest.fn(),
  },
  garden: {
    findFirst: jest.fn(),
    update: jest.fn(),
  },
  soil: {
    create: jest.fn(),
  },
  plantInstance: {
    create: jest.fn(),
    deleteMany: jest.fn(),
  },
};

const mockPerenual = {
  importSpecies: jest.fn(),
};

describe('SpeciesService', () => {
  let service: SpeciesService;

  beforeEach(async () => {
    jest.clearAllMocks();

    const module: TestingModule = await Test.createTestingModule({
      providers: [
        SpeciesService,
        { provide: DatabaseService, useValue: mockDb },
        { provide: PerenualService, useValue: mockPerenual },
      ],
    }).compile();

    service = module.get<SpeciesService>(SpeciesService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });

  describe('saveSpecies', () => {
    it('computes and stores bloomDays on the species record', async () => {
      mockPerenual.importSpecies.mockResolvedValue(mockSpecies);
      mockDb.species.update.mockResolvedValue({ ...mockSpecies, bloomDays: 60 });
      mockDb.user.update.mockResolvedValue({});

      const result = await service.saveSpecies(1, 100);

      expect(mockDb.species.update).toHaveBeenCalledWith({
        where: { id: 1 },
        data: { bloomDays: expect.any(Number) },
      });
      expect(result.species.bloomDays).toBeGreaterThan(0);
      expect(result.message).toBe('Species saved successfully');
    });

    it('links the species to the user', async () => {
      mockPerenual.importSpecies.mockResolvedValue(mockSpecies);
      mockDb.species.update.mockResolvedValue({ ...mockSpecies, bloomDays: 60 });
      mockDb.user.update.mockResolvedValue({});

      await service.saveSpecies(1, 100);

      expect(mockDb.user.update).toHaveBeenCalledWith({
        where: { id: 1 },
        data: { savedSpecies: { connect: { id: 1 } } },
      });
    });
  });

  describe('saveSpeciesToGarden', () => {
    const mockGarden = { id: 5, ownerId: 1 };

    it('connects the species to the garden saved list without creating a plant instance', async () => {
      mockDb.garden.findFirst.mockResolvedValue(mockGarden);
      mockPerenual.importSpecies.mockResolvedValue(mockSpecies);
      mockDb.species.update.mockResolvedValue({ ...mockSpecies, bloomDays: 60 });
      mockDb.garden.update.mockResolvedValue({});

      const result = await service.saveSpeciesToGarden(1, 100, 5);

      expect(mockDb.garden.update).toHaveBeenCalledWith({
        where: { id: 5 },
        data: { savedSpecies: { connect: { id: 1 } } },
      });
      expect(mockDb.plantInstance.create).not.toHaveBeenCalled();
      expect(mockDb.soil.create).not.toHaveBeenCalled();
      expect(result.species.bloomDays).toBeGreaterThan(0);
    });

    it('throws NotFoundException when garden does not belong to user', async () => {
      mockDb.garden.findFirst.mockResolvedValue(null);

      await expect(service.saveSpeciesToGarden(1, 100, 99)).rejects.toThrow(
        NotFoundException,
      );
      expect(mockDb.plantInstance.create).not.toHaveBeenCalled();
      expect(mockDb.garden.update).not.toHaveBeenCalled();
    });
  });

  describe('unsaveSpecies', () => {
    it('disconnects the species from the user', async () => {
      mockDb.species.findUnique.mockResolvedValue(mockSpecies);
      mockDb.user.update.mockResolvedValue({});

      const result = await service.unsaveSpecies(1, 100);

      expect(mockDb.user.update).toHaveBeenCalledWith({
        where: { id: 1 },
        data: { savedSpecies: { disconnect: { id: 1 } } },
      });
      expect(result.message).toBe('Species unsaved successfully');
    });

    it('throws NotFoundException when species is not in database', async () => {
      mockDb.species.findUnique.mockResolvedValue(null);

      await expect(service.unsaveSpecies(1, 999)).rejects.toThrow(NotFoundException);
    });
  });

  describe('unsaveSpeciesFromGarden', () => {
    it('disconnects the species from the garden saved list without touching plant instances', async () => {
      mockDb.species.findUnique.mockResolvedValue(mockSpecies);
      mockDb.garden.findFirst.mockResolvedValue({ id: 5, ownerId: 1 });
      mockDb.garden.update.mockResolvedValue({});

      const result = await service.unsaveSpeciesFromGarden(1, 100, 5);

      expect(mockDb.garden.update).toHaveBeenCalledWith({
        where: { id: 5 },
        data: { savedSpecies: { disconnect: { id: 1 } } },
      });
      expect(mockDb.plantInstance.deleteMany).not.toHaveBeenCalled();
      expect(result.message).toBe('Species removed from garden');
    });

    it('throws NotFoundException when species is not in database', async () => {
      mockDb.species.findUnique.mockResolvedValue(null);

      await expect(service.unsaveSpeciesFromGarden(1, 999, 5)).rejects.toThrow(
        NotFoundException,
      );
    });

    it('throws NotFoundException when garden does not belong to user', async () => {
      mockDb.species.findUnique.mockResolvedValue(mockSpecies);
      mockDb.garden.findFirst.mockResolvedValue(null);

      await expect(service.unsaveSpeciesFromGarden(1, 100, 99)).rejects.toThrow(
        NotFoundException,
      );
    });
  });

  describe('getSavedSpecies', () => {
    it('returns species with modelCategory and bloomDays', async () => {
      mockDb.user.findUnique.mockResolvedValue({
        id: 1,
        savedSpecies: [{ ...mockSpecies, bloomDays: 60 }],
      });

      const result = await service.getSavedSpecies(1);

      expect(result[0].modelCategory).toBe('flower');
      expect(result[0].bloomDays).toBe(60);
    });

    it('computes bloomDays on the fly when not stored in DB', async () => {
      mockDb.user.findUnique.mockResolvedValue({
        id: 1,
        savedSpecies: [{ ...mockSpecies, bloomDays: null }],
      });

      const result = await service.getSavedSpecies(1);

      expect(result[0].bloomDays).toBeGreaterThan(0);
    });

    it('assigns modelCategory vegetable for edible fruit species', async () => {
      mockDb.user.findUnique.mockResolvedValue({
        id: 1,
        savedSpecies: [{ ...mockSpecies, type: 'Fruit', edibleFruit: true, bloomDays: 80 }],
      });

      const result = await service.getSavedSpecies(1);

      expect(result[0].modelCategory).toBe('vegetable');
    });

    it('assigns modelCategory tree for tree species', async () => {
      mockDb.user.findUnique.mockResolvedValue({
        id: 1,
        savedSpecies: [{ ...mockSpecies, type: 'Tree', bloomDays: 730 }],
      });

      const result = await service.getSavedSpecies(1);

      expect(result[0].modelCategory).toBe('tree');
    });

    it('throws NotFoundException when user does not exist', async () => {
      mockDb.user.findUnique.mockResolvedValue(null);

      await expect(service.getSavedSpecies(999)).rejects.toThrow(NotFoundException);
    });
  });

  describe('getSavedSpeciesForGarden', () => {
    it('returns species from the garden saved list', async () => {
      mockDb.garden.findFirst.mockResolvedValue({
        id: 5,
        ownerId: 1,
        savedSpecies: [
          { ...mockSpecies, bloomDays: 60 },
          { ...mockSpecies, id: 2, commonName: 'Crape Myrtle', type: 'Tree', bloomDays: 730 },
        ],
      });

      const result = await service.getSavedSpeciesForGarden(1, 5);

      expect(result).toHaveLength(2);
      expect(result[0].bloomDays).toBe(60);
      expect(result[1].modelCategory).toBe('tree');
    });

    it('throws NotFoundException when garden does not belong to user', async () => {
      mockDb.garden.findFirst.mockResolvedValue(null);

      await expect(service.getSavedSpeciesForGarden(1, 99)).rejects.toThrow(
        NotFoundException,
      );
    });
  });
});
