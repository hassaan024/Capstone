from fastapi import APIRouter
from ..schema import AverageRequest, AverageResponse
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


# all routes in this router will start with /predict
router = APIRouter(
    prefix="/predict",
    tags=["prediction"]
)

@router.get("/test")
async def test():
    return {"status": "ok"}

@router.post("/average", response_model=AverageResponse)
async def predict_using_mean(request: AverageRequest) -> AverageResponse:
    logger.info(f"HIITTTTING")
    numbers: list[float] = request.numbers
    n: int = len(numbers)
    prediction = sum(numbers) / n
    logger.info(f"Prediction: {prediction}")
    return AverageResponse(average=prediction)