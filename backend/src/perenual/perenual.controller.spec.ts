import { Test, TestingModule } from '@nestjs/testing';
import { PerenualController } from './perenual.controller';

describe('PerenualController', () => {
  let controller: PerenualController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [PerenualController],
    }).compile();

    controller = module.get<PerenualController>(PerenualController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
