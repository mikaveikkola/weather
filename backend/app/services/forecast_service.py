import logging
from datetime import datetime, timedelta, timezone
from typing import List

from sqlalchemy import func, select, text
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


async def get_forecasts_history_by_place(
    db: AsyncSession,
    place_name: str,
    model: str = "harmonie",
    hours: int = 48,
) -> List[Forecast]:
    """Return the most recently-made forecast for each past valid_time.

    Uses DISTINCT ON (valid_time) to pick, for every past hour, the forecast
    that was fetched most recently — so the comparison tab can match them
    against actual observations.
    """
    stmt = text(
        """
        SELECT DISTINCT ON (valid_time)
            id, fetched_at, valid_time, fmisid, place_name,
            latitude, longitude, model,
            temperature, wind_speed, wind_direction, wind_gust,
            precipitation_1h, humidity, pressure, cloud_cover, dew_point
        FROM forecasts
        WHERE place_name = :place
          AND model      = :model
          AND valid_time >= :cutoff
          AND valid_time <= :now
        ORDER BY valid_time, fetched_at DESC
        """
    )
    now = datetime.now(timezone.utc)
    cutoff = now - timedelta(hours=hours)
    result = await db.execute(
        stmt, {"place": place_name, "model": model, "cutoff": cutoff, "now": now}
    )
    rows = result.mappings().all()
    forecasts = []
    for r in rows:
        f = Forecast()
        for col in r.keys():
            setattr(f, col, r[col])
        forecasts.append(f)
    return forecasts
