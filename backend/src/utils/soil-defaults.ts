import { SoilType } from 'enums/table_enums';

export interface SoilDefaults {
  pH: number;
  nitrogen: number;
  phosphorus: number;
  potassium: number;
  organicPercentage: number;
}

export const SOIL_DEFAULTS: Record<SoilType, SoilDefaults> = {
  LOAM:  { pH: 6.5, nitrogen: 40, phosphorus: 30, potassium: 35, organicPercentage: 4.0 },
  SANDY: { pH: 6.0, nitrogen: 15, phosphorus: 10, potassium: 12, organicPercentage: 1.0 },
  CLAY:  { pH: 7.0, nitrogen: 50, phosphorus: 40, potassium: 45, organicPercentage: 3.0 },
  SILT:  { pH: 6.5, nitrogen: 35, phosphorus: 25, potassium: 30, organicPercentage: 2.5 },
  PEAT:  { pH: 4.5, nitrogen: 20, phosphorus: 10, potassium: 15, organicPercentage: 8.0 },
  CHALK: { pH: 8.0, nitrogen: 20, phosphorus: 25, potassium: 20, organicPercentage: 1.5 },
};

export function getSoilDefaults(type: SoilType): SoilDefaults {
  return SOIL_DEFAULTS[type];
}