import { Test, TestingModule } from '@nestjs/testing';
import { PlantInstanceController } from './plant-instance.controller';
import { PlantInstanceService } from './plant-instance.service';

describe('PlantInstanceController', () => {
  let controller: PlantInstanceController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [PlantInstanceController],
      providers: [{ provide: PlantInstanceService, useValue: {} }],
    }).compile();

    controller = module.get<PlantInstanceController>(PlantInstanceController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
