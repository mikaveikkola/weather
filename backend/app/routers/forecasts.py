from typing import Optional

from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.ext.asyncio import AsyncSession

from ..database import get_db
from ..schemas.forecast import ForecastRead, ForecastResponse
from ..schemas.station import StationRead
from ..services import forecast_service, station_service

router = APIRouter()


@router.get("", response_model=ForecastResponse)
async def get_forecasts(
    fmisid: Optional[int] = Query(None, description="Station FMISID"),
    place: Optional[str] = Query(None, description="Place name (e.g. Helsinki)"),
    model: str = Query("harmonie", pattern="^(harmonie|ecmwf)$"),
    hours: int = Query(48, ge=1, le=240),
    db: AsyncSession = Depends(get_db),
):
    station = None

    if fmisid:
        station = await station_service.get_station(db, fmisid)
        if not station:
            raise HTTPException(status_code=404, detail="Station not found")
        forecasts = await forecast_service.get_forecasts(db, fmisid, model, hours)
    elif place:
        forecasts = await forecast_service.get_forecasts_by_place(db, place, model)
    else:
        raise HTTPException(status_code=400, detail="Either fmisid or place is required")

    fetched_at = forecasts[0].fetched_at if forecasts else None
    return ForecastResponse(
        station=StationRead.model_validate(station) if station else None,
        model=model,
        fetched_at=fetched_at,
        data=[ForecastRead.model_validate(f) for f in forecasts],
    )
