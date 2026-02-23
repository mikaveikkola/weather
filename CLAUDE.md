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
