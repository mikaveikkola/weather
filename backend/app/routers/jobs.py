import logging
from typing import Any, Dict, List, Optional

from fastapi import APIRouter, BackgroundTasks, Depends, HTTPException, Query, Request
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from ..database import get_db
from ..models.fetch_log import FetchLog
from ..scheduler.jobs import fetch_harmonie_job, fetch_observations_job

logger = logging.getLogger(__name__)
router = APIRouter()


@router.get("/status")
async def job_status(request: Request) -> List[Dict[str, Any]]:
    scheduler = getattr(request.app.state, "scheduler", None)
    if not scheduler:
        return []
    jobs = []
    for job in scheduler.get_jobs():
        jobs.append(
            {
                "id": job.id,
                "name": job.name,
                "next_run": job.next_run_time.isoformat() if job.next_run_time else None,
                "trigger": str(job.trigger),
            }
        )
    return jobs


@router.post("/trigger")
async def trigger_job(
    body: Dict[str, str],
    background_tasks: BackgroundTasks,
):
    job_name = body.get("job")
    if job_name == "observations":
        background_tasks.add_task(fetch_observations_job)
        return {"status": "triggered", "job": job_name}
    elif job_name == "forecast_harmonie":
        background_tasks.add_task(fetch_harmonie_job)
        return {"status": "triggered", "job": job_name}
    else:
        raise HTTPException(status_code=400, detail=f"Unknown job: {job_name}")


@router.get("/log")
async def job_log(
    job_type: Optional[str] = Query(None),
    limit: int = Query(50, ge=1, le=500),
    db: AsyncSession = Depends(get_db),
) -> List[Dict[str, Any]]:
    stmt = select(FetchLog).order_by(FetchLog.started_at.desc()).limit(limit)
    if job_type:
        stmt = stmt.where(FetchLog.job_type == job_type)
    result = await db.execute(stmt)
    logs = result.scalars().all()
    return [
        {
            "id": log.id,
            "job_type": log.job_type,
            "status": log.status,
            "started_at": log.started_at.isoformat() if log.started_at else None,
            "finished_at": log.finished_at.isoformat() if log.finished_at else None,
            "records_fetched": log.records_fetched,
            "error_message": log.error_message,
        }
        for log in logs
    ]
