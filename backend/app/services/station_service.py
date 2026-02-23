import logging
from typing import List, Optional

from sqlalchemy import select
from sqlalchemy.dialects.postgresql import insert
from sqlalchemy.ext.asyncio import AsyncSession

from ..models.station import Station

logger = logging.getLogger(__name__)


async def get_all_stations(db: AsyncSession, active_only: bool = True) -> List[Station]:
    stmt = select(Station)
    if active_only:
        stmt = stmt.where(Station.is_active.is_(True))
    stmt = stmt.order_by(Station.name)
    result = await db.execute(stmt)
    return list(result.scalars().all())


async def get_station(db: AsyncSession, fmisid: int) -> Optional[Station]:
    result = await db.execute(select(Station).where(Station.fmisid == fmisid))
    return result.scalar_one_or_none()


async def upsert_stations(db: AsyncSession, stations: List[dict]) -> int:
    if not stations:
        return 0
    stmt = insert(Station).values(stations)
    stmt = stmt.on_conflict_do_update(
        index_elements=["fmisid"],
        set_={
            "name": stmt.excluded.name,
            "region": stmt.excluded.region,
            "latitude": stmt.excluded.latitude,
            "longitude": stmt.excluded.longitude,
            "is_active": stmt.excluded.is_active,
        },
    )
    await db.execute(stmt)
    await db.commit()
    return len(stations)
