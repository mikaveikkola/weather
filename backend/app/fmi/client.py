import asyncio
import logging
from datetime import datetime
from functools import partial

from fmiopendata.wfs import download_stored_query

from . import stored_queries as sq

logger = logging.getLogger(__name__)


async def _run_in_executor(func):
    loop = asyncio.get_event_loop()
    return await loop.run_in_executor(None, func)


async def fetch_observations(starttime: datetime, endtime: datetime, bbox: str):
    """Fetch multipointcoverage observations for a bounding box."""
    args = [
        f"starttime={starttime.strftime('%Y-%m-%dT%H:%M:%SZ')}",
        f"endtime={endtime.strftime('%Y-%m-%dT%H:%M:%SZ')}",
        f"bbox={bbox}",
        "timestep=10",
    ]
    logger.info(f"Fetching observations {starttime.isoformat()} - {endtime.isoformat()}")
    func = partial(download_stored_query, sq.OBSERVATIONS_MULTIPOINTCOVERAGE, args=args)
    return await _run_in_executor(func)


async def fetch_forecast_harmonie(place: str):
    """Fetch Harmonie surface point forecast for a named place."""
    args = [f"place={place}"]
    logger.info(f"Fetching Harmonie forecast for {place}")
    func = partial(download_stored_query, sq.FORECAST_HARMONIE, args=args)
    return await _run_in_executor(func)
