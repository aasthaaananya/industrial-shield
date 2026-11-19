from fastapi import APIRouter, Query
from database import get_connection

router = APIRouter(prefix="/api/alerts", tags=["Alerts"])

@router.get("")
def list_alerts(limit: int = Query(default=50, ge=1, le=500)):
    with get_connection() as connection:
        rows = connection.execute(
            "SELECT * FROM alerts ORDER BY id DESC LIMIT ?", (limit,)
        ).fetchall()
    return [dict(row) for row in rows]
