import { Injectable, NestInterceptor, ExecutionContext, CallHandler } from '@nestjs/common';
import { Observable } from 'rxjs';
import { tap } from 'rxjs/operators';
import { DatabaseService } from './database.service';

const WRITE_METHODS = new Set(['POST', 'PATCH', 'PUT', 'DELETE']);

@Injectable()
export class LogDbInterceptor implements NestInterceptor {
  constructor(private readonly db: DatabaseService) {}

  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    const req = context.switchToHttp().getRequest<{ method: string; url: string }>();

    if (!WRITE_METHODS.has(req.method)) return next.handle();

    return next.handle().pipe(
      tap(async () => {
        console.log(`\n[DB CHANGE] ${req.method} ${req.url}`);
        await this.db.printDatabase();
      }),
    );
  }
}
