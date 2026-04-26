import { Test, TestingModule } from '@nestjs/testing';
import { PerenualService } from './perenual.service';
import { DatabaseService } from 'database/database.service';

describe('PerenualService', () => {
  let service: PerenualService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        PerenualService,
        { provide: DatabaseService, useValue: { species: {} } },
      ],
    }).compile();

    service = module.get<PerenualService>(PerenualService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
