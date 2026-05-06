from fastapi import FastAPI
from .routers import predict
import os
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

logger.info("Current working directory: %s", os.getcwd())

# initalize app
app = FastAPI(title="Model API")

# Include all the modules
app.include_router(predict.router)