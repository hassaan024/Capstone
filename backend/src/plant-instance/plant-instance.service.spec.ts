import { Test, TestingModule } from '@nestjs/testing';
import { PlantInstanceService } from './plant-instance.service.js';

describe('PlantInstanceService', () => {
  let service: PlantInstanceService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [PlantInstanceService],
    }).compile();

    service = module.get<PlantInstanceService>(PlantInstanceService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
