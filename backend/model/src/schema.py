from pydantic import BaseModel
from typing import List, Optional


class PlantInput(BaseModel):
    id: int
    heightCm: float
    ageDays: int
    daysAhead: int
    healthStatus: Optional[str] = None   # Healthy | Wilting | Sick | Dead | NeedsWater
    growthStage: Optional[str] = None    # Seedling | Vegetative | Flowering | Fruiting | Dormant
    bloomState: bool = False


class SpeciesInput(BaseModel):
    growthRate: float                    # 1=Low, 2=Medium, 3=High
    minHeightCm: float
    maxHeightCm: float
    avgHoursSun: Optional[float] = None
    minTemp: Optional[float] = None      # °F
    maxTemp: Optional[float] = None      # °F
    wateringMinDays: Optional[int] = None
    wateringMaxDays: Optional[int] = None
    droughtTolerant: bool = False
    tropical: bool = False
    cycle: Optional[str] = None


class SoilInput(BaseModel):
    type: str                            # LOAM | SANDY | CLAY | SILT | PEAT | CHALK
    pH: float
    nitrogen: float
    phosphorus: float
    potassium: float
    organicPercentage: Optional[float] = None
    estimatedMoisture: float             # 0–1


class WeatherDay(BaseModel):
    date: Optional[str] = None
    temperatureF: float
    precipitation: float                 # mm
    sunlightIntensity: float             # MJ/m²
    dailyEvaporation: float             # mm
    vapourPressureDeficit: float
    relativeHumidity: float
    windSpeedMph: float


class PredictionRequest(BaseModel):
    plant: PlantInput
    species: SpeciesInput
    soil: SoilInput
    weather: List[WeatherDay]


# ── Response ────────────────────────────────────────────────────────────────

class StressFactors(BaseModel):
    temperature: float   # 0–1
    sunlight: float      # 0–1
    water: float         # 0–1
    soil: float          # 0–1
    overall: float       # product of all factors


class PredictionResponse(BaseModel):
    plantInstanceId: int
    currentHeightCm: float
    predictedHeightCm: float
    growthCm: float
    daysAhead: int
    dailyGrowthCm: float
    stressFactors: StressFactors
    confidence: float    # 0–1, lower when data is sparse or extreme
    notes: List[str]
