# Project settings for Claude Code

## Git
- Repo: https://github.com/mikaveikkola/weather
- Branch: main
- Push: `git push` (token stored in ~/.git-credentials)
- Commit identity: mikaveikkola / mikaveikkola@users.noreply.github.com

## Services
- Frontend: http://localhost:3000 (React + TypeScript, nginx proxy)
- Python backend: http://localhost:8000 (FastAPI)
- C++ backend: http://localhost:8001 (Drogon)
- Database: PostgreSQL + TimescaleDB, port 5432

## Docker
- Start all: `docker compose up`
- Rebuild frontend: `docker compose build frontend && docker compose up -d frontend`
- Rebuild C++ backend: `docker compose build backend-cpp && docker compose up -d backend-cpp`

## nginx (frontend/nginx.conf)
- `/api/` proxied to `http://backend:8000` (Python backend)
- To switch to C++: change to `http://backend-cpp:8001`
- Frontend has a Python/C++ toggle button in the UI (Header.tsx)

## C++ backend (backend-cpp/)
- Language: C++20, framework: Drogon v1.9.6
- Libraries: libpqxx 7.9.2 (built from source), libcurl, pugixml
- WfsParser uses tree-walking (not XPath) — pugixml local-name() returns full prefixed name
- Field maps: StoredQueries.h (obs: short codes t2m/ws_10min/..., forecasts: CamelCase Temperature/WindSpeedMS/...)

## Python backend (backend/)
- Framework: FastAPI + SQLAlchemy (async)
- DB migrations: Alembic (`backend/alembic/versions/`)

## Frontend (frontend/)
- Backend selection stored in localStorage key `backend` ('python' | 'cpp')
- Context: src/contexts/BackendContext.tsx
- API base URLs: /api/v1 (Python via nginx) or http://localhost:8001/api/v1 (C++ direct)

## Frontend views (Dashboard.tsx)
Three tabs:
- **Havainnot** — observation time series charts (temperature+dew point, precipitation+snow, wind)
  - Selectable time range: 6h / 24h / 48h / 7d / 14d / 30d
  - Components: TemperatureChart, PrecipitationChart, WindChart
- **Ennuste** — 48h Harmonie model forecast table (ForecastTable)
- **Vertailu** — comparison of past forecasts vs actual observations
  - Chart: ComparisonChart (Recharts ComposedChart, dual Y-axis)
    - Blue solid line = observation, orange dashed = forecast
    - Coloured bars on right axis = error (green = fc too low, red = fc too high)
  - Table: ComparisonTable (colour-coded diff cells)
  - Data: uses `/forecasts/history` endpoint, NOT `/forecasts`
  - Observations fetched with useObservations(fmisid, 48) → obs48h

## API endpoints (both backends, /api/v1/)
- GET /health
- GET /stations, /stations/{fmisid}, /stations/{fmisid}/latest
- GET /observations?fmisid=X&hours=N
- GET /observations/latest, /observations/summary
- GET /forecasts?place=Helsinki&model=harmonie
- GET /forecasts/history?place=Helsinki&hours=48  ← past forecasts for Vertailu tab
  - Uses DISTINCT ON (valid_time) ORDER BY valid_time, fetched_at DESC
  - Returns the most recently-made forecast for each past valid_time
- GET /jobs/status, POST /jobs/trigger, GET /jobs/log

## Python backend — forecast history (forecast_service.py)
- `get_forecasts_history_by_place()` uses SQLAlchemy `text()` with raw SQL
- Pass cutoff as Python `datetime` object (asyncpg can't handle interval arithmetic in SQL params)
- Pattern: `{"cutoff": datetime.now(utc) - timedelta(hours=hours), "now": datetime.now(utc)}`

## C++ backend — forecast history (ForecastService.cpp)
- `getForecastsHistoryByPlace()` uses `($3 || ' hours')::INTERVAL` with hours as int
- Route registered BEFORE the base /forecasts route to avoid Drogon path conflicts
