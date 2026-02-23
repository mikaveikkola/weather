from sqlalchemy import Column, Integer, String, Float, Boolean, DateTime, Text
from sqlalchemy.sql import func

from ..database import Base


class Station(Base):
    __tablename__ = "stations"

    fmisid = Column(Integer, primary_key=True)
    name = Column(Text, nullable=False)
    region = Column(Text)
    country = Column(Text, default="Finland")
    latitude = Column(Float, nullable=False)
    longitude = Column(Float, nullable=False)
    elevation = Column(Float)
    station_type = Column(Text)
    is_active = Column(Boolean, default=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), server_default=func.now(), onupdate=func.now())
