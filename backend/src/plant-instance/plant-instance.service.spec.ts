import { Test, TestingModule } from '@nestjs/testing';
import { PlantInstanceService } from './plant-instance.service';
import { DatabaseService } from 'database/database.service';

describe('PlantInstanceService', () => {
  let service: PlantInstanceService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        PlantInstanceService,
        { provide: DatabaseService, useValue: { plantInstance: {}, soil: {}, species: {} } },
      ],
    }).compile();

    service = module.get<PlantInstanceService>(PlantInstanceService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
