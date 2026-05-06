import { Test, TestingModule } from '@nestjs/testing';
import { SoilService } from './soil.service';
import { DatabaseService } from 'database/database.service';

describe('SoilService', () => {
  let service: SoilService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        SoilService,
        { provide: DatabaseService, useValue: { soil: {} } },
      ],
    }).compile();

    service = module.get<SoilService>(SoilService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
