from datetime import datetime, timedelta, timezone
from typing import List, Optional

from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.ext.asyncio import AsyncSession

from ..database import get_db
from ..schemas.observation import ObservationSummary
from ..schemas.station import StationRead
from ..services import observation_service, station_service

router = APIRouter()


@router.get("")
async def get_observations(
    fmisid: int = Query(..., description="Station FMISID"),
    start: Optional[datetime] = Query(None),
    end: Optional[datetime] = Query(None),
    resolution: str = Query("auto", pattern="^(raw|hourly|daily|auto)$"),
    db: AsyncSession = Depends(get_db),
):
    station = await station_service.get_station(db, fmisid)
    if not station:
        raise HTTPException(status_code=404, detail="Station not found")

    if end is None:
        end = datetime.now(timezone.utc)
    if start is None:
        start = end - timedelta(hours=24)

    # Ensure timezone aware
    if start.tzinfo is None:
        start = start.replace(tzinfo=timezone.utc)
    if end.tzinfo is None:
        end = end.replace(tzinfo=timezone.utc)

    data = await observation_service.get_observations(db, fmisid, start, end, resolution)
    return {
        "station": StationRead.model_validate(station),
        "resolution": resolution,
        "data": data,
    }


@router.get("/latest")
async def get_latest(
    fmisids: Optional[str] = Query(None, description="Comma-separated FMISID list"),
    db: AsyncSession = Depends(get_db),
):
    fmisid_list: Optional[List[int]] = None
    if fmisids:
        try:
            fmisid_list = [int(x.strip()) for x in fmisids.split(",")]
        except ValueError:
            raise HTTPException(status_code=400, detail="Invalid fmisids parameter")

    return await observation_service.get_latest_observations(db, fmisid_list)


@router.get("/summary", response_model=ObservationSummary)
async def get_summary(
    fmisid: int = Query(...),
    period: str = Query("24h", pattern="^(24h|7d|30d)$"),
    db: AsyncSession = Depends(get_db),
):
    station = await station_service.get_station(db, fmisid)
    if not station:
        raise HTTPException(status_code=404, detail="Station not found")

    summary = await observation_service.get_summary(db, fmisid, period)
    return ObservationSummary(fmisid=fmisid, period=period, **summary)
