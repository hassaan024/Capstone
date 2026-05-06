import {
  PipeTransform,
  Injectable,
  BadRequestException,
  Logger,
} from '@nestjs/common';

const logger = new Logger('WeatherService');

// create type from literal array
@Injectable()
export class ValidateLocationPipe implements PipeTransform {
  transform(value: string | number | undefined) {
    logger.log(value);

    if (!value) {
      throw new BadRequestException('Latitude or Longitude is undefined');
    }

    if (isNaN(Number(value))) {
      throw new BadRequestException(
        'Latitude and longitude must be valid numbers',
      );
    }

    return value;
  }
}
