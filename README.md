# FMI Säätiedot

Finnish weather application using open data from the Finnish Meteorological Institute (FMI).

## Architecture

```
Browser
  │
  └── Frontend (React + Vite)  :3000
        │  nginx reverse proxy
        ├──/api/v1/──► Python backend (FastAPI)  :8000
        │               │
        └── direct ──► C++ backend (Drogon)      :8001
                        │
                Both backends ──► PostgreSQL + TimescaleDB  :5432
```

The header has a **Python / C++** toggle to switch backends at runtime. Both backends expose the same REST API and share the same database.

### Services

| Service | Technology | Port | Description |
|---------|-----------|------|-------------|
| Frontend | React + TypeScript + Vite, nginx | 3000 | SPA, proxies `/api/` to Python backend |
| Backend (Python) | FastAPI + SQLAlchemy + asyncpg | 8000 | Primary backend |
| Backend (C++) | Drogon + libpqxx + libcurl + pugixml | 8001 | Parallel implementation |
| Database | PostgreSQL 16 + TimescaleDB | 5432 | Time-series optimised storage |

### Data flow

1. **Scheduler** (both backends, independent) polls FMI WFS API every 10 minutes for observations and 60 minutes for Harmonie model forecasts
2. Observations are stored per station and per 10-minute interval; forecasts per place and valid hour
3. `ON CONFLICT DO NOTHING` prevents duplicates when both backends write simultaneously
4. Frontend fetches data via REST API and renders charts using Recharts

### API endpoints

All endpoints are prefixed with `/api/v1/`.

| Method | Path | Description |
|--------|------|-------------|
| GET | /health | Service health, DB status |
| GET | /stations | All stations (`?active=true`) |
| GET | /stations/{fmisid} | Single station |
| GET | /stations/{fmisid}/latest | Station + latest observation |
| GET | /observations | Time series (`?fmisid=X&hours=24`) |
| GET | /observations/latest | Latest per station |
| GET | /observations/summary | Min/max/avg stats |
| GET | /forecasts | Future forecast (`?place=Helsinki`) |
| GET | /forecasts/history | Past forecasts for obs comparison (`?place=Helsinki&hours=48`) |
| GET | /jobs/status | Scheduler status |
| POST | /jobs/trigger | Trigger manual fetch |
| GET | /jobs/log | Fetch log |

### Frontend views

- **Havainnot** — observation time series (temperature, precipitation, wind), selectable 6h–30d range
- **Ennuste** — 48-hour Harmonie model forecast table
- **Vertailu** — side-by-side chart and table comparing past forecasts against actual observations, with colour-coded error

## Running

```bash
docker compose up
```

Open http://localhost:3000

## Requirements

- Docker + Docker Compose
