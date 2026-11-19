from datetime import datetime, timezone
from fastapi import APIRouter, HTTPException, Query
from database import get_connection
from models import SensorReadingIn
from risk_engine import assess_risk

router = APIRouter(prefix="/api/sensors", tags=["Sensors"])

@router.post("/readings")
def receive_reading(payload: SensorReadingIn):
    timestamp = payload.timestamp or datetime.now(timezone.utc).isoformat()
    risk = assess_risk(payload.gas_value, payload.temperature)

    with get_connection() as connection:
        previous = connection.execute(
            "SELECT risk_level FROM readings ORDER BY id DESC LIMIT 1"
        ).fetchone()

        cursor = connection.execute("""
            INSERT INTO readings
            (gas_value, temperature, relay_on, buzzer_on, risk_level, risk_reason, timestamp)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (
            payload.gas_value, payload.temperature, int(payload.relay_on),
            int(payload.buzzer_on), risk["level"], risk["reason"], timestamp
        ))
        reading_id = cursor.lastrowid

        if risk["level"] != "SAFE" and (previous is None or previous["risk_level"] != risk["level"]):
            connection.execute(
                "INSERT INTO alerts (level, message, timestamp) VALUES (?, ?, ?)",
                (risk["level"], risk["reason"], timestamp)
            )
        connection.commit()

    return {
        "id": reading_id,
        "gas_value": payload.gas_value,
        "temperature": payload.temperature,
        "relay_on": payload.relay_on,
        "buzzer_on": payload.buzzer_on,
        "risk": risk,
        "risk_level": risk["level"],
        "risk_reason": risk["reason"],
        "timestamp": timestamp
    }

@router.get("/latest")
def latest_reading():
    with get_connection() as connection:
        row = connection.execute("SELECT * FROM readings ORDER BY id DESC LIMIT 1").fetchone()
    if row is None:
        raise HTTPException(status_code=404, detail="No sensor readings received yet")
    result = dict(row)
    result["relay_on"] = bool(result["relay_on"])
    result["buzzer_on"] = bool(result["buzzer_on"])
    result["risk"] = {"level": result["risk_level"], "description": result["risk_reason"]}
    return result

@router.get("/history")
def reading_history(limit: int = Query(default=30, ge=1, le=500)):
    with get_connection() as connection:
        rows = connection.execute(
            "SELECT * FROM readings ORDER BY id DESC LIMIT ?", (limit,)
        ).fetchall()
    return [dict(row) for row in reversed(rows)]
