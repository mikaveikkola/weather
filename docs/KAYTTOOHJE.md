# Käyttöohje – FMI Säätiedot

## Esivaatimukset

- [Docker](https://www.docker.com/) ja Docker Compose asennettuna
- Internet-yhteys (FMI:n avoin data)

---

## Pikaopas – Docker-käynnistys

```bash
# 1. Siirry projektin hakemistoon
cd weather/

# 2. Kopioi ympäristömuuttujatiedosto
cp .env.example .env

# 3. Muokkaa tarvittaessa (vaihda salasana!)
nano .env

# 4. Käynnistä kaikki palvelut
docker-compose up --build -d

# 5. Tarkista että palvelut käynnistyivät
docker-compose ps

# 6. Avaa selaimessa
# Frontend: http://localhost:3000
# Backend API: http://localhost:8000/docs
```

---

## Kehitysympäristö (ilman Dockeria)

### Tietokanta (TimescaleDB tai PostgreSQL)

```bash
# PostgreSQL esimerkki
docker run -d \
  --name weather-db \
  -e POSTGRES_DB=weather \
  -e POSTGRES_USER=weather \
  -e POSTGRES_PASSWORD=weather \
  -p 5432:5432 \
  timescale/timescaledb:latest-pg16
```

### Backend

```bash
cd backend/

# Asenna riippuvuudet
pip install -r requirements.txt

# Luo .env tiedosto
echo "DATABASE_URL=postgresql+asyncpg://weather:weather@localhost:5432/weather" > .env

# Aja tietokantamigraatiot
alembic upgrade head

# Käynnistä kehityspalvelin
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

### Frontend

```bash
cd frontend/

# Asenna riippuvuudet
npm install

# Käynnistä kehityspalvelin (proxy -> localhost:8000)
npm run dev

# Avaa: http://localhost:3000
```

---

## API-dokumentaatio

FastAPI generoi automaattisesti interaktiivisen dokumentaation:
- **Swagger UI**: http://localhost:8000/docs
- **ReDoc**: http://localhost:8000/redoc

---

## Historian lataus (backfill)

Kun sovellus käynnistyy ensimmäistä kertaa, se alkaa kerätä tietoja automaattisesti.
Historian lataamiseen voit käyttää API:a:

```bash
# Käynnistä havainnot manuaalisesti
curl -X POST http://localhost:8000/api/v1/jobs/trigger \
  -H "Content-Type: application/json" \
  -d '{"job": "observations"}'

# Käynnistä ennusteen haku
curl -X POST http://localhost:8000/api/v1/jobs/trigger \
  -H "Content-Type: application/json" \
  -d '{"job": "forecast_harmonie"}'
```

---

## Seuranta ja lokit

```bash
# Tarkista ajastustöiden tila
curl http://localhost:8000/api/v1/jobs/status

# Tarkista hakuhistoria
curl http://localhost:8000/api/v1/jobs/log

# Tarkista järjestelmän tila
curl http://localhost:8000/api/v1/health

# Docker-lokit
docker-compose logs -f backend
docker-compose logs -f db
```

---

## Tietokantakomennot

```bash
# Yhdistä tietokantaan
docker-compose exec db psql -U weather -d weather

# Alembic-migraatiot
cd backend/
alembic upgrade head      # aja uudet migraatiot
alembic history           # näytä historia
alembic current           # näytä nykytila

# Tarkista tietoja
SELECT COUNT(*) FROM observations;
SELECT * FROM stations ORDER BY name LIMIT 10;
SELECT * FROM fetch_log ORDER BY started_at DESC LIMIT 5;
```

---

## Vianetsintä

| Ongelma | Ratkaisu |
|---------|---------|
| Backend ei käynnisty | Tarkista tietokantayhteys: `docker-compose logs backend` |
| Ei havaintotietoja | Odota ~10 min tai käynnistä manuaalisesti `/api/v1/jobs/trigger` |
| FMI-haku epäonnistuu | Tarkista internet-yhteys, FMI:n palvelut voivat olla tilapäisesti poissa |
| Frontend ei näy | Tarkista että frontend-kontti käynnistyi: `docker-compose ps` |
| Tietokantavirhe | Aja migraatiot: `alembic upgrade head` |

---

## Palveluiden sammuttaminen

```bash
# Sammuta palvelut (tiedot säilyvät)
docker-compose stop

# Sammuta ja poista kontit (tiedot säilyvät volumessa)
docker-compose down

# Sammuta ja poista myös tietokantadata
docker-compose down -v
```
