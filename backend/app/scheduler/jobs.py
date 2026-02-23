import logging
from datetime import datetime, timedelta, timezone

from ..config import settings
from ..database import AsyncSessionLocal
from ..fmi import client as fmi_client
from ..fmi import parsers
from ..models.fetch_log import FetchLog
from ..services import forecast_service, observation_service, station_service

logger = logging.getLogger(__name__)


async def fetch_observations_job() -> None:
    """Fetch the latest 20-minute window of observations from FMI."""
    now = datetime.now(timezone.utc)
    starttime = now - timedelta(minutes=20)
    endtime = now

    async with AsyncSessionLocal() as db:
        log = FetchLog(
            job_type="observations",
            status="running",
            query_params={
                "bbox": settings.fmi_bbox,
                "start": starttime.isoformat(),
                "end": endtime.isoformat(),
            },
        )
        db.add(log)
        await db.commit()
        await db.refresh(log)

        try:
            result = await fmi_client.fetch_observations(starttime, endtime, settings.fmi_bbox)

            station_dicts = parsers.parse_stations(result.location_metadata)
            await station_service.upsert_stations(db, station_dicts)

            obs_rows = parsers.parse_observations(result.data, result.location_metadata)
            count = await observation_service.upsert_observations(db, obs_rows)

            log.status = "success"
            log.records_fetched = count
            log.finished_at = datetime.now(timezone.utc)
            await db.commit()
            logger.info(f"Observations fetched: {count} records")
        except Exception as exc:
            logger.error(f"Observation fetch failed: {exc}")
            log.status = "error"
            log.error_message = str(exc)
            log.finished_at = datetime.now(timezone.utc)
            await db.commit()


async def fetch_harmonie_job() -> None:
    """Fetch Harmonie forecasts for all configured places."""
    now = datetime.now(timezone.utc)

    for place in settings.places_list:
        async with AsyncSessionLocal() as db:
            log = FetchLog(
                job_type="forecast_harmonie",
                status="running",
                query_params={"place": place},
            )
            db.add(log)
            await db.commit()
            await db.refresh(log)

            try:
                result = await fmi_client.fetch_forecast_harmonie(place)
                rows = parsers.parse_forecast_harmonie(
                    result.data, result.location_metadata, now, place
                )
                count = await forecast_service.upsert_forecasts(db, rows)

                log.status = "success"
                log.records_fetched = count
                log.finished_at = datetime.now(timezone.utc)
                await db.commit()
                logger.info(f"Harmonie forecast for {place}: {count} records")
            except Exception as exc:
                logger.error(f"Harmonie forecast failed for {place}: {exc}")
                log.status = "error"
                log.error_message = str(exc)
                log.finished_at = datetime.now(timezone.utc)
                await db.commit()
