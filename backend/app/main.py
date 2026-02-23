import logging
from contextlib import asynccontextmanager
from datetime import datetime, timezone

from apscheduler.schedulers.asyncio import AsyncIOScheduler
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .config import settings
from .database import Base, engine
from .models import FetchLog, Forecast, Observation, Station  # noqa: F401 – register models
from .routers import forecasts, health, jobs, observations, stations
from .scheduler.jobs import fetch_harmonie_job, fetch_observations_job

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(name)s: %(message)s")
logger = logging.getLogger(__name__)

scheduler = AsyncIOScheduler(timezone="UTC")


@asynccontextmanager
async def lifespan(app: FastAPI):
    # Create tables (fallback if alembic has not been run)
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)

    # Schedule jobs
    scheduler.add_job(
        fetch_observations_job,
        "interval",
        minutes=settings.fetch_interval_minutes,
        id="obs_fetch",
        max_instances=1,
        coalesce=True,
        next_run_time=datetime.now(timezone.utc),  # run immediately on startup
    )
    scheduler.add_job(
        fetch_harmonie_job,
        "interval",
        minutes=settings.forecast_interval_minutes,
        id="fc_harmonie",
        max_instances=1,
        coalesce=True,
    )
    scheduler.start()
    logger.info("Scheduler started")

    app.state.scheduler = scheduler

    yield

    scheduler.shutdown()
    await engine.dispose()


app = FastAPI(
    title="FMI Weather API",
    description="Säätietoja FMI:n avoimesta datasta",
    version="1.0.0",
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(health.router, prefix="/api/v1", tags=["health"])
app.include_router(stations.router, prefix="/api/v1/stations", tags=["stations"])
app.include_router(observations.router, prefix="/api/v1/observations", tags=["observations"])
app.include_router(forecasts.router, prefix="/api/v1/forecasts", tags=["forecasts"])
app.include_router(jobs.router, prefix="/api/v1/jobs", tags=["jobs"])
