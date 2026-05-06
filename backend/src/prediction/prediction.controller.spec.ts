import { Test, TestingModule } from '@nestjs/testing';
import { PredictionController } from './prediction.controller';
import { PredictionService } from './prediction.service';

const mockService = {
  predict: jest.fn(),
  generateTimeline: jest.fn(),
  seedDemoSpecies: jest.fn(),
};

describe('PredictionController', () => {
  let controller: PredictionController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [PredictionController],
      providers: [{ provide: PredictionService, useValue: mockService }],
    }).compile();

    controller = module.get<PredictionController>(PredictionController);
    jest.clearAllMocks();
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });

  it('predict delegates to service', () => {
    mockService.predict.mockResolvedValue({ predictedHeightCm: 10 });
    controller.predict(1, 7);
    expect(mockService.predict).toHaveBeenCalledWith(1, 7);
  });

  it('generateTimeline parses bloomDate and delegates to service', () => {
    mockService.generateTimeline.mockResolvedValue({ plants: [] });
    controller.generateTimeline(5, { bloomDate: '2027-06-01' });
    expect(mockService.generateTimeline).toHaveBeenCalledWith(
      5,
      new Date('2027-06-01'),
    );
  });

  it('seedDemoSpecies delegates to service', () => {
    mockService.seedDemoSpecies.mockResolvedValue({ species: [] });
    controller.seedDemoSpecies();
    expect(mockService.seedDemoSpecies).toHaveBeenCalled();
  });
});
