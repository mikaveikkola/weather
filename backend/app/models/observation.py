from sqlalchemy import Column, Integer, Float, SmallInteger, DateTime

from ..database import Base


class Observation(Base):
    __tablename__ = "observations"

    time = Column(DateTime(timezone=True), primary_key=True, nullable=False)
    fmisid = Column(Integer, primary_key=True, nullable=False)
    temperature = Column(Float)
    dew_point = Column(Float)
    humidity = Column(Float)
    wind_speed = Column(Float)
    wind_gust = Column(Float)
    wind_direction = Column(SmallInteger)
    precipitation_1h = Column(Float)
    precip_intensity = Column(Float)
    snow_depth = Column(Float)
    pressure = Column(Float)
    visibility = Column(Integer)
    cloud_cover = Column(SmallInteger)
    weather_code = Column(SmallInteger)
