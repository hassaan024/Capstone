import { Test, TestingModule } from '@nestjs/testing';
import { SpeciesController } from './species.controller.js';
import { SpeciesService } from './species.service.js';

describe('SpeciesController', () => {
  let controller: SpeciesController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [SpeciesController],
      providers: [SpeciesService],
    }).compile();

    controller = module.get<SpeciesController>(SpeciesController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
