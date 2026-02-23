# FMI Säätiedot

Finnish weather application using open data from the Finnish Meteorological Institute (FMI).

## Architecture

| Service | Technology | Port |
|---------|-----------|------|
| Frontend | React + TypeScript + Vite | 3000 |
| Backend (Python) | FastAPI + SQLAlchemy | 8000 |
| Backend (C++) | Drogon + libpqxx | 8001 |
| Database | PostgreSQL + TimescaleDB | 5432 |

The frontend includes a **Python / C++** toggle in the header to switch between backends at runtime.

## Features

- Real-time weather observations from FMI weather stations across Finland
- 50-hour Harmonie model forecasts for major cities
- Temperature, wind, humidity, pressure, precipitation, snow depth
- Time-series charts with selectable time ranges (6h – 30d)
- Automatic data fetching every 10 minutes (observations) and 60 minutes (forecasts)

## Running

```bash
docker compose up
```

Open [http://localhost:3000](http://localhost:3000)

## Requirements

- Docker + Docker Compose
