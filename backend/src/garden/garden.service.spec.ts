import { Test, TestingModule } from '@nestjs/testing';
import { GardenService } from './garden.service';
import { DatabaseService } from 'database/database.service';

describe('GardenService', () => {
  let service: GardenService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        GardenService,
        { provide: DatabaseService, useValue: { garden: {} } },
      ],
    }).compile();

    service = module.get<GardenService>(GardenService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
