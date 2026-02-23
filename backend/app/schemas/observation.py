from pydantic import BaseModel, ConfigDict
from typing import Optional, List
from datetime import datetime

from .station import StationRead


class ObservationRead(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    time: datetime
    fmisid: int
    temperature: Optional[float] = None
    dew_point: Optional[float] = None
    humidity: Optional[float] = None
    wind_speed: Optional[float] = None
    wind_gust: Optional[float] = None
    wind_direction: Optional[int] = None
    precipitation_1h: Optional[float] = None
    precip_intensity: Optional[float] = None
    snow_depth: Optional[float] = None
    pressure: Optional[float] = None
    visibility: Optional[int] = None
    cloud_cover: Optional[int] = None
    weather_code: Optional[int] = None


class ObservationResponse(BaseModel):
    station: StationRead
    resolution: str
    data: List[dict]


class ObservationSummary(BaseModel):
    fmisid: int
    period: str
    min_temperature: Optional[float] = None
    max_temperature: Optional[float] = None
    avg_temperature: Optional[float] = None
    avg_humidity: Optional[float] = None
    avg_wind_speed: Optional[float] = None
    max_wind_gust: Optional[float] = None
    total_precipitation: Optional[float] = None
    avg_pressure: Optional[float] = None
