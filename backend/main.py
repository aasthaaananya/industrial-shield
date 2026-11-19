from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from database import initialize_database
from routers import sensors, alerts, devices

@asynccontextmanager
async def lifespan(app: FastAPI):
    initialize_database()
    yield

app = FastAPI(
    title="Industrial Shield API",
    description="Prototype API for industrial hazard monitoring",
    version="1.0.0",
    lifespan=lifespan
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Restrict this to your frontend origin before deployment.
    allow_credentials=False,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(sensors.router)
app.include_router(alerts.router)
app.include_router(devices.router)

@app.get("/")
def root():
    return {"message": "Industrial Shield API is running"}

@app.get("/api/system/health")
def system_health():
    return {"status": "online", "service": "industrial-shield-api"}
