import { Test, TestingModule } from '@nestjs/testing';
import { GardenController } from './garden.controller.js';
import { GardenService } from './garden.service.js';

describe('GardenController', () => {
  let controller: GardenController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [GardenController],
      providers: [GardenService],
    }).compile();

    controller = module.get<GardenController>(GardenController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
