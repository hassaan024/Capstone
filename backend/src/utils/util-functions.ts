export function celsiusToFahrenheit(celsius: number): number {
  return (celsius * 9) / 5 + 32;
}
export function metersPerSecondToMph(metersPerSecond: number): number {
  const mph = metersPerSecond * 2.23694;
  return Math.round(mph * 1000) / 1000;
}
