import logging
from datetime import datetime, timezone
from typing import List

from sqlalchemy import func, select
from sqlalchemy.dialects.postgresql import insert
from sqlalchemy.ext.asyncio import AsyncSession

from ..models.forecast import Forecast

logger = logging.getLogger(__name__)


async def upsert_forecasts(db: AsyncSession, rows: List[dict]) -> int:
    if not rows:
        return 0
    stmt = insert(Forecast).values(rows)
    stmt = stmt.on_conflict_do_nothing()
    await db.execute(stmt)
    await db.commit()
    return len(rows)


async def get_forecasts(
    db: AsyncSession,
    fmisid: int,
    model: str = "harmonie",
    hours: int = 48,
) -> List[Forecast]:
    now = datetime.now(timezone.utc)

    latest_fetch_result = await db.execute(
        select(func.max(Forecast.fetched_at)).where(
            Forecast.fmisid == fmisid,
            Forecast.model == model,
        )
    )
    latest_at = latest_fetch_result.scalar()
    if not latest_at:
        return []

    stmt = (
        select(Forecast)
        .where(
            Forecast.fmisid == fmisid,
            Forecast.model == model,
            Forecast.fetched_at == latest_at,
            Forecast.valid_time >= now,
        )
        .order_by(Forecast.valid_time)
    )
    result = await db.execute(stmt)
    return list(result.scalars().all())


async def get_forecasts_by_place(
    db: AsyncSession,
    place_name: str,
    model: str = "harmonie",
) -> List[Forecast]:
    now = datetime.now(timezone.utc)

    latest_fetch_result = await db.execute(
        select(func.max(Forecast.fetched_at)).where(
            Forecast.place_name == place_name,
            Forecast.model == model,
        )
    )
    latest_at = latest_fetch_result.scalar()
    if not latest_at:
        return []

    stmt = (
        select(Forecast)
        .where(
            Forecast.place_name == place_name,
            Forecast.model == model,
            Forecast.fetched_at == latest_at,
            Forecast.valid_time >= now,
        )
        .order_by(Forecast.valid_time)
    )
    result = await db.execute(stmt)
    return list(result.scalars().all())
