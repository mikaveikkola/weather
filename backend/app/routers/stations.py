from typing import List

from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession

from ..database import get_db
from ..schemas.station import StationLatest, StationRead
from ..services import observation_service, station_service

router = APIRouter()


@router.get("", response_model=List[StationRead])
async def list_stations(
    active: bool = True,
    db: AsyncSession = Depends(get_db),
):
    stations = await station_service.get_all_stations(db, active_only=active)
    return stations


@router.get("/{fmisid}", response_model=StationRead)
async def get_station(fmisid: int, db: AsyncSession = Depends(get_db)):
    station = await station_service.get_station(db, fmisid)
    if not station:
        raise HTTPException(status_code=404, detail="Station not found")
    return station


@router.get("/{fmisid}/latest", response_model=StationLatest)
async def get_station_latest(fmisid: int, db: AsyncSession = Depends(get_db)):
    station = await station_service.get_station(db, fmisid)
    if not station:
        raise HTTPException(status_code=404, detail="Station not found")
    obs = await observation_service.get_latest_observation(db, fmisid)
    return StationLatest(
        fmisid=station.fmisid,
        name=station.name,
        region=station.region,
        latitude=station.latitude,
        longitude=station.longitude,
        time=obs.time if obs else None,
        temperature=obs.temperature if obs else None,
        humidity=obs.humidity if obs else None,
        wind_speed=obs.wind_speed if obs else None,
        wind_direction=obs.wind_direction if obs else None,
        wind_gust=obs.wind_gust if obs else None,
        precipitation_1h=obs.precipitation_1h if obs else None,
        snow_depth=obs.snow_depth if obs else None,
        pressure=obs.pressure if obs else None,
    )
