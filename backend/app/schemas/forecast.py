from pydantic import BaseModel, ConfigDict
from typing import Optional, List
from datetime import datetime

from .station import StationRead


class ForecastRead(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    id: int
    fetched_at: datetime
    valid_time: datetime
    fmisid: Optional[int] = None
    place_name: Optional[str] = None
    model: str
    temperature: Optional[float] = None
    wind_speed: Optional[float] = None
    wind_direction: Optional[int] = None
    wind_gust: Optional[float] = None
    precipitation_1h: Optional[float] = None
    humidity: Optional[float] = None
    pressure: Optional[float] = None
    cloud_cover: Optional[float] = None
    dew_point: Optional[float] = None


class ForecastResponse(BaseModel):
    station: Optional[StationRead] = None
    model: str
    fetched_at: Optional[datetime] = None
    data: List[ForecastRead]
