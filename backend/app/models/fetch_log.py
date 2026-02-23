from sqlalchemy import Column, BigInteger, Text, Integer, DateTime
from sqlalchemy.dialects.postgresql import JSONB
from sqlalchemy.sql import func

from ..database import Base


class FetchLog(Base):
    __tablename__ = "fetch_log"

    id = Column(BigInteger, primary_key=True, autoincrement=True)
    started_at = Column(DateTime(timezone=True), server_default=func.now())
    finished_at = Column(DateTime(timezone=True))
    job_type = Column(Text, nullable=False)
    status = Column(Text, nullable=False)
    records_fetched = Column(Integer, default=0)
    error_message = Column(Text)
    query_params = Column(JSONB)
