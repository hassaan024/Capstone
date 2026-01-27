import { PipeTransform, Injectable, BadRequestException } from '@nestjs/common';

// create type from literal array
const allowedOrigins = ['react', 'unreal'] as const;
export type Origin = (typeof allowedOrigins)[number];

@Injectable()
export class OriginValidationPipe implements PipeTransform {
  transform(value: unknown): Origin {
    if (
      typeof value !== 'string' ||
      !allowedOrigins.includes(value as Origin)
    ) {
      throw new BadRequestException(
        `Origin must be one of: ${allowedOrigins.join(', ')}`,
      );
    }

    return value as Origin;
  }
}
