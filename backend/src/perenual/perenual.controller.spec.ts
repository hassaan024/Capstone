import { Test, TestingModule } from '@nestjs/testing';
import { PerenualController } from './perenual.controller';
import { PerenualService } from './perenual.service';

describe('PerenualController', () => {
  let controller: PerenualController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [PerenualController],
      providers: [{ provide: PerenualService, useValue: {} }],
    }).compile();

    controller = module.get<PerenualController>(PerenualController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
