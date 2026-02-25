from pydantic import BaseModel

# -----------------------Schemas----------------------------
class AverageRequest(BaseModel):
    numbers: list[float]
class AverageResponse(BaseModel):
    average: float 