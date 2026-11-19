from pydantic import BaseModel, Field
from typing import Optional

class SensorReadingIn(BaseModel):
    gas_value: float = Field(ge=0, le=100000)
    temperature: float = Field(ge=-100, le=300)
    relay_on: bool = False
    buzzer_on: bool = False
    timestamp: Optional[str] = None

class SensorReadingOut(BaseModel):
    id: int
    gas_value: float
    temperature: float
    relay_on: bool
    buzzer_on: bool
    risk_level: str
    risk_reason: str
    timestamp: str
