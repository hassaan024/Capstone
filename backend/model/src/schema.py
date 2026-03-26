from pydantic import BaseModel
from typing import List, Optional, Dict

# -----------------------Schemas----------------------------
class AverageRequest(BaseModel):
    numbers: list[float]

class AverageResponse(BaseModel):
    average: float 

class WeatherDay(BaseModel):
    date: str
    temp: float
    rain: float
    humidity: Optional[float] = None

class Location(BaseModel):
    lat: float
    lon: float

class PredictionRequest(BaseModel):
    plant: Dict  # you can refine later
    weather: List[WeatherDay]
    soil: Optional[Dict] = None
    dates: List[str]
    location: Location

class Prediction(BaseModel):
    date: str
    value: float

class PredictionResponse(BaseModel):
    predictions: List[Prediction]