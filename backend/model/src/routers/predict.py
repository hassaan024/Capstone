from fastapi import APIRouter
from ..schema import PredictionRequest, PredictionResponse
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


# Main prediction endpoint
@router.post("/", response_model=PredictionResponse)
async def predict(request: PredictionRequest):
    logger.info("Prediction request received")
    
    # Dummy logic for now
    results = [{"date": date, "value": 0.85} for date in request.dates]
    
    logger.info(f"Returning predictions: {results}")
    return {"predictions": results}