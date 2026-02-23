from datetime import datetime, timezone
from typing import Any, Dict

from fastapi import APIRouter, Depends
from sqlalchemy import select, func
from sqlalchemy.ext.asyncio import AsyncSession

from ..database import get_db
from ..models.observation import Observation
from ..models.fetch_log import FetchLog

router = APIRouter()


@router.get("/health")
async def health(db: AsyncSession = Depends(get_db)) -> Dict[str, Any]:
    db_connected = True
    last_observation: str | None = None
    last_fetch: str | None = None

    try:
        result = await db.execute(select(func.max(Observation.time)))
        ts = result.scalar()
        if ts:
            last_observation = ts.isoformat()

        log_result = await db.execute(
            select(FetchLog.finished_at)
            .where(FetchLog.status == "success")
            .order_by(FetchLog.finished_at.desc())
            .limit(1)
        )
        log_ts = log_result.scalar()
        if log_ts:
            last_fetch = log_ts.isoformat()
    except Exception:
        db_connected = False

    return {
        "status": "ok" if db_connected else "degraded",
        "db_connected": db_connected,
        "last_observation": last_observation,
        "last_fetch": last_fetch,
        "server_time": datetime.now(timezone.utc).isoformat(),
    }
