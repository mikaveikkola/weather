from pydantic import BaseModel, ConfigDict
from typing import Optional
from datetime import datetime


class StationRead(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    fmisid: int
    name: str
    region: Optional[str] = None
    country: Optional[str] = "Finland"
    latitude: float
    longitude: float
    elevation: Optional[float] = None
    station_type: Optional[str] = None
    is_active: bool = True


class StationLatest(BaseModel):
    fmisid: int
    name: str
    region: Optional[str] = None
    latitude: float
    longitude: float
    time: Optional[datetime] = None
    temperature: Optional[float] = None
    humidity: Optional[float] = None
    wind_speed: Optional[float] = None
    wind_direction: Optional[int] = None
    wind_gust: Optional[float] = None
    precipitation_1h: Optional[float] = None
    snow_depth: Optional[float] = None
    pressure: Optional[float] = None
