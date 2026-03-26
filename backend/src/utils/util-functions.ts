export function celsiusToFahrenheit(celsius: number): number {
  return (celsius * 9) / 5 + 32;
}
export function metersPerSecondToMph(metersPerSecond: number): number {
  const mph = metersPerSecond * 2.23694;
  return Math.round(mph * 1000) / 1000;
}
export function inchesToFeet(inches: number): number {
  return  inches / 12;
}
export function metersToFeet(meters: number): number {
  return meters * 3.28084;
}
export function cmToFeet(cm: number): number {
  return cm * 0.0328084;
}
export function mmToFeet(mm: number): number {
  return mm * 0.00328084;
}