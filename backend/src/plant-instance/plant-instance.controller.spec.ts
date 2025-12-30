import { Test, TestingModule } from '@nestjs/testing';
import { PlantInstanceController } from './plant-instance.controller.js';
import { PlantInstanceService } from './plant-instance.service.js';

describe('PlantInstanceController', () => {
  let controller: PlantInstanceController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [PlantInstanceController],
      providers: [PlantInstanceService],
    }).compile();

    controller = module.get<PlantInstanceController>(PlantInstanceController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
