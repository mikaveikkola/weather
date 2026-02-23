import logging
from datetime import datetime, timedelta, timezone
from typing import Any, Dict, List, Optional

from sqlalchemy import func, select, text
from sqlalchemy.dialects.postgresql import insert
from sqlalchemy.ext.asyncio import AsyncSession

from ..models.observation import Observation
from ..models.station import Station

logger = logging.getLogger(__name__)


async def upsert_observations(db: AsyncSession, rows: List[dict]) -> int:
    if not rows:
        return 0
    stmt = insert(Observation).values(rows)
    stmt = stmt.on_conflict_do_nothing(index_elements=["time", "fmisid"])
    await db.execute(stmt)
    await db.commit()
    return len(rows)


async def get_observations(
    db: AsyncSession,
    fmisid: int,
    start: datetime,
    end: datetime,
    resolution: str = "auto",
) -> List[Dict[str, Any]]:
    duration = end - start
    if resolution == "auto":
        if duration <= timedelta(hours=24):
            resolution = "raw"
        elif duration <= timedelta(days=7):
            resolution = "hourly"
        else:
            resolution = "daily"

    if resolution == "raw":
        stmt = (
            select(Observation)
            .where(Observation.fmisid == fmisid, Observation.time >= start, Observation.time <= end)
            .order_by(Observation.time)
        )
        result = await db.execute(stmt)
        return [
            {
                "time": o.time,
                "temperature": o.temperature,
                "dew_point": o.dew_point,
                "humidity": o.humidity,
                "wind_speed": o.wind_speed,
                "wind_gust": o.wind_gust,
                "wind_direction": o.wind_direction,
                "precipitation_1h": o.precipitation_1h,
                "snow_depth": o.snow_depth,
                "pressure": o.pressure,
                "visibility": o.visibility,
                "cloud_cover": o.cloud_cover,
            }
            for o in result.scalars().all()
        ]

    bucket = "1 hour" if resolution == "hourly" else "1 day"
    sql = text(
        """
        SELECT
            time_bucket(:bucket, time) AS time,
            AVG(temperature)        AS temperature,
            AVG(dew_point)          AS dew_point,
            AVG(humidity)           AS humidity,
            AVG(wind_speed)         AS wind_speed,
            MAX(wind_gust)          AS wind_gust,
            AVG(wind_direction)     AS wind_direction,
            SUM(precipitation_1h)   AS precipitation_1h,
            AVG(snow_depth)         AS snow_depth,
            AVG(pressure)           AS pressure,
            AVG(visibility)         AS visibility,
            AVG(cloud_cover)        AS cloud_cover
        FROM observations
        WHERE fmisid = :fmisid AND time >= :start AND time <= :end
        GROUP BY 1
        ORDER BY 1
        """
    )
    try:
        result = await db.execute(sql, {"bucket": bucket, "fmisid": fmisid, "start": start, "end": end})
        return [dict(row._mapping) for row in result]
    except Exception:
        # Fallback: raw data if TimescaleDB time_bucket not available
        return await get_observations(db, fmisid, start, end, "raw")


async def get_latest_observation(db: AsyncSession, fmisid: int) -> Optional[Observation]:
    stmt = (
        select(Observation)
        .where(Observation.fmisid == fmisid)
        .order_by(Observation.time.desc())
        .limit(1)
    )
    result = await db.execute(stmt)
    return result.scalar_one_or_none()


async def get_latest_observations(
    db: AsyncSession,
    fmisids: Optional[List[int]] = None,
) -> List[Dict[str, Any]]:
    subq = select(Observation.fmisid, func.max(Observation.time).label("max_time"))
    if fmisids:
        subq = subq.where(Observation.fmisid.in_(fmisids))
    subq = subq.group_by(Observation.fmisid).subquery()

    stmt = (
        select(Observation, Station)
        .join(subq, (Observation.fmisid == subq.c.fmisid) & (Observation.time == subq.c.max_time))
        .join(Station, Station.fmisid == Observation.fmisid)
    )
    result = await db.execute(stmt)
    return [
        {
            "fmisid": obs.fmisid,
            "name": station.name,
            "region": station.region,
            "latitude": station.latitude,
            "longitude": station.longitude,
            "time": obs.time,
            "temperature": obs.temperature,
            "humidity": obs.humidity,
            "wind_speed": obs.wind_speed,
            "wind_direction": obs.wind_direction,
            "wind_gust": obs.wind_gust,
            "precipitation_1h": obs.precipitation_1h,
            "snow_depth": obs.snow_depth,
            "pressure": obs.pressure,
        }
        for obs, station in result.all()
    ]


async def get_summary(db: AsyncSession, fmisid: int, period: str = "24h") -> Dict[str, Any]:
    hours = {"24h": 24, "7d": 168, "30d": 720}.get(period, 24)
    sql = text(
        """
        SELECT
            MIN(temperature)        AS min_temperature,
            MAX(temperature)        AS max_temperature,
            AVG(temperature)        AS avg_temperature,
            AVG(humidity)           AS avg_humidity,
            AVG(wind_speed)         AS avg_wind_speed,
            MAX(wind_gust)          AS max_wind_gust,
            SUM(precipitation_1h)   AS total_precipitation,
            AVG(pressure)           AS avg_pressure
        FROM observations
        WHERE fmisid = :fmisid
          AND time >= NOW() - (:hours * INTERVAL '1 hour')
        """
    )
    result = await db.execute(sql, {"fmisid": fmisid, "hours": hours})
    row = result.fetchone()
    return dict(row._mapping) if row else {}
