import asyncio
import logging
from datetime import datetime, timedelta, timezone

from ..config import settings
from ..database import AsyncSessionLocal
from ..fmi import client as fmi_client
from ..fmi import parsers
from ..services import observation_service, station_service

logger = logging.getLogger(__name__)


async def backfill_observations(days_back: int = 7) -> None:
    """Fetch historical observations going back N days, one day at a time."""
    logger.info(f"Starting backfill for {days_back} days")
    now = datetime.now(timezone.utc).replace(minute=0, second=0, microsecond=0)

    for i in range(days_back):
        chunk_end = now - timedelta(days=i)
        chunk_start = chunk_end - timedelta(days=1)

        logger.info(f"Backfilling {chunk_start.date()} → {chunk_end.date()}")
        try:
            result = await fmi_client.fetch_observations(chunk_start, chunk_end, settings.fmi_bbox)
            async with AsyncSessionLocal() as db:
                station_dicts = parsers.parse_stations(result.location_metadata)
                await station_service.upsert_stations(db, station_dicts)
                obs_rows = parsers.parse_observations(result.data, result.location_metadata)
                count = await observation_service.upsert_observations(db, obs_rows)
                logger.info(f"Backfilled {count} records for {chunk_start.date()}")
        except Exception as exc:
            logger.error(f"Backfill failed for {chunk_start.date()}: {exc}")

        await asyncio.sleep(2)  # Be polite to FMI service

    logger.info("Backfill complete")
