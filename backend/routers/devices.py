from fastapi import APIRouter
from database import get_connection

router = APIRouter(prefix="/api/devices", tags=["Devices"])

@router.get("/status")
def device_status():
    with get_connection() as connection:
        row = connection.execute(
            "SELECT relay_on, buzzer_on, timestamp FROM readings ORDER BY id DESC LIMIT 1"
        ).fetchone()

    if row is None:
        return {"esp32": "NO_DATA", "relay_on": False, "buzzer_on": False, "last_seen": None}

    return {
        "esp32": "REPORTED",
        "relay_on": bool(row["relay_on"]),
        "buzzer_on": bool(row["buzzer_on"]),
        "last_seen": row["timestamp"]
    }
