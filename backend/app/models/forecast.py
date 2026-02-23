from sqlalchemy import Column, BigInteger, Integer, Float, SmallInteger, DateTime, Text

from ..database import Base


class Forecast(Base):
    __tablename__ = "forecasts"

    id = Column(BigInteger, primary_key=True, autoincrement=True)
    fetched_at = Column(DateTime(timezone=True), nullable=False)
    valid_time = Column(DateTime(timezone=True), nullable=False)
    fmisid = Column(Integer)
    place_name = Column(Text)
    latitude = Column(Float)
    longitude = Column(Float)
    model = Column(Text, nullable=False)
    temperature = Column(Float)
    wind_speed = Column(Float)
    wind_direction = Column(SmallInteger)
    wind_gust = Column(Float)
    precipitation_1h = Column(Float)
    humidity = Column(Float)
    pressure = Column(Float)
    cloud_cover = Column(Float)
    dew_point = Column(Float)
