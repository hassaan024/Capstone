import { Test, TestingModule } from '@nestjs/testing';
import { GardenController } from './garden.controller';
import { GardenService } from './garden.service';

describe('GardenController', () => {
  let controller: GardenController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [GardenController],
      providers: [{ provide: GardenService, useValue: {} }],
    }).compile();

    controller = module.get<GardenController>(GardenController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
