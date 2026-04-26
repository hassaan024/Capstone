import { Test, TestingModule } from '@nestjs/testing';
import { SpeciesService } from './species.service';
import { DatabaseService } from 'database/database.service';
import { PerenualService } from 'perenual/perenual.service';

describe('SpeciesService', () => {
  let service: SpeciesService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        SpeciesService,
        { provide: DatabaseService, useValue: { species: {} } },
        { provide: PerenualService, useValue: {} },
      ],
    }).compile();

    service = module.get<SpeciesService>(SpeciesService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
