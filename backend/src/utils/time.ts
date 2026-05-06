import { Logger } from "@nestjs/common";

const logger = new Logger('Date Manager');

export function daysToMilliseconds(days: number): number{
    return (days * 24 * 60 * 60 * 1000);
}

export function millisecondsToDateString(milliseconds: number): string {
    return new Date(milliseconds).toISOString().split('T')[0];
}

export function dateStringToMilliseconds(date: string): number {
    return (new Date(date).getTime())
}

export function getTodaysDateAsString(): string {
    const today: Date = new Date();
    const todays_date: string = millisecondsToDateString(today.getTime());
    return todays_date;
}

export function calculatePastDate(
    offset: number, // how far back to go in units of page_size
    page_size: number, // number of days in a page
    start_date: string = getTodaysDateAsString()
): string {
    const days_skipped: number = offset * page_size;
    const start: Date = new Date(start_date);
    const start_milliseconds: number = start.getTime();
    const milliseconds_back: number = daysToMilliseconds(days_skipped)
    const end_milliseconds: number = start_milliseconds - milliseconds_back
    const end_date: string = millisecondsToDateString(end_milliseconds)
    logger.log(`
        ====================================
        Offset: ${offset},
        Page_Size: ${page_size},
        Start Date: ${start_date},
        Days_skipped: ${days_skipped},
        Start: ${start},
        Start_milli: ${start_milliseconds},
        Milli_back: ${milliseconds_back},
        End_milli: ${end_milliseconds},
        End_Date: ${end_date}
        ====================================
    `)
    return end_date;
}

export function goBackDays(date: string, days: number): string {
    const final_millisecs: number = dateStringToMilliseconds(date) - daysToMilliseconds(days);
    return millisecondsToDateString(final_millisecs);
}

export function goForwardDays(date: string, days: number): string {
    const final_millisecs: number = dateStringToMilliseconds(date) + daysToMilliseconds(days);
    return millisecondsToDateString(final_millisecs);
}

