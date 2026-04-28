import { Test, TestingModule } from '@nestjs/testing';
import { SpeciesController } from './species.controller';
import { SpeciesService } from './species.service';

const mockService = {
  getSavedSpecies: jest.fn(),
  getSavedSpeciesForGarden: jest.fn(),
  saveSpecies: jest.fn(),
  saveSpeciesToGarden: jest.fn(),
  unsaveSpecies: jest.fn(),
  unsaveSpeciesFromGarden: jest.fn(),
  create: jest.fn(),
  findAll: jest.fn(),
  findOne: jest.fn(),
  update: jest.fn(),
  remove: jest.fn(),
};

describe('SpeciesController', () => {
  let controller: SpeciesController;

  beforeEach(async () => {
    jest.clearAllMocks();

    const module: TestingModule = await Test.createTestingModule({
      controllers: [SpeciesController],
      providers: [{ provide: SpeciesService, useValue: mockService }],
    }).compile();

    controller = module.get<SpeciesController>(SpeciesController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });

  describe('getSaved', () => {
    it('calls getSavedSpecies when no gardenId is provided', () => {
      mockService.getSavedSpecies.mockResolvedValue([]);
      controller.getSaved('1');
      expect(mockService.getSavedSpecies).toHaveBeenCalledWith(1);
      expect(mockService.getSavedSpeciesForGarden).not.toHaveBeenCalled();
    });

    it('calls getSavedSpeciesForGarden when gardenId is provided', () => {
      mockService.getSavedSpeciesForGarden.mockResolvedValue([]);
      controller.getSaved('1', '5');
      expect(mockService.getSavedSpeciesForGarden).toHaveBeenCalledWith(1, 5);
      expect(mockService.getSavedSpecies).not.toHaveBeenCalled();
    });
  });

  describe('saveSpecies', () => {
    it('calls saveSpecies when no gardenId is provided', () => {
      mockService.saveSpecies.mockResolvedValue({ message: 'Species saved successfully' });
      controller.saveSpecies('100', '1');
      expect(mockService.saveSpecies).toHaveBeenCalledWith(1, 100);
      expect(mockService.saveSpeciesToGarden).not.toHaveBeenCalled();
    });

    it('calls saveSpeciesToGarden when gardenId is provided', () => {
      mockService.saveSpeciesToGarden.mockResolvedValue({ message: 'Species added to garden' });
      controller.saveSpecies('100', '1', '5');
      expect(mockService.saveSpeciesToGarden).toHaveBeenCalledWith(1, 100, 5);
      expect(mockService.saveSpecies).not.toHaveBeenCalled();
    });
  });

  describe('unsaveSpecies', () => {
    it('calls unsaveSpecies when no gardenId is provided', () => {
      mockService.unsaveSpecies.mockResolvedValue({ message: 'Species unsaved successfully' });
      controller.unsaveSpecies('100', '1');
      expect(mockService.unsaveSpecies).toHaveBeenCalledWith(1, 100);
      expect(mockService.unsaveSpeciesFromGarden).not.toHaveBeenCalled();
    });

    it('calls unsaveSpeciesFromGarden when gardenId is provided', () => {
      mockService.unsaveSpeciesFromGarden.mockResolvedValue({ message: 'Removed 1 plant(s) from garden' });
      controller.unsaveSpecies('100', '1', '5');
      expect(mockService.unsaveSpeciesFromGarden).toHaveBeenCalledWith(1, 100, 5);
      expect(mockService.unsaveSpecies).not.toHaveBeenCalled();
    });
  });

  describe('findAll', () => {
    it('delegates to service.findAll', () => {
      mockService.findAll.mockReturnValue([]);
      controller.findAll();
      expect(mockService.findAll).toHaveBeenCalled();
    });
  });

  describe('findOne', () => {
    it('converts string id to number', () => {
      mockService.findOne.mockReturnValue(null);
      controller.findOne('42');
      expect(mockService.findOne).toHaveBeenCalledWith(42);
    });
  });

  describe('remove', () => {
    it('converts string id to number', () => {
      mockService.remove.mockResolvedValue({});
      controller.remove('7');
      expect(mockService.remove).toHaveBeenCalledWith(7);
    });
  });
});
