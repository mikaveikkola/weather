# Arkkitehtuuri – FMI Säätiedot

## Järjestelmäkuvaus

FMI Säätiedot on kolmitasoinen web-sovellus, joka hakee automaattisesti säätietoja
Ilmatieteen laitoksen (FMI) avoimesta WFS-rajapinnasta, tallentaa ne aikasarjatietokantaan
ja esittää ne selainpohjaisella käyttöliittymällä.

---

## Arkkitehtuurikaavio

```
┌─────────────────────────────────────────────────────────┐
│                      SELAIN                             │
│            React 18 + TypeScript + Recharts             │
│                   http://localhost:3000                  │
└────────────────────────┬────────────────────────────────┘
                         │ REST API JSON (/api/v1/...)
                         │ Nginx-proxy (Docker-tuotanto)
┌────────────────────────▼────────────────────────────────┐
│                    FASTAPI BACKEND                       │
│                   http://localhost:8000                  │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │ REST API -reitittimet                             │  │
│  │ /stations  /observations  /forecasts  /jobs       │  │
│  └──────────────────┬───────────────────────────────┘  │
│                     │                                    │
│  ┌──────────────────▼───────────────────────────────┐  │
│  │ Palvelukerros                                     │  │
│  │ StationService  ObservationService  ForecastSvc   │  │
│  └───────┬──────────────────────────────────────────┘  │
│          │                                               │
│  ┌───────▼──────┐    ┌──────────────────────────────┐  │
│  │  SQLAlchemy  │    │       APScheduler             │  │
│  │  ORM (async) │    │  obs: joka 10 min             │  │
│  └───────┬──────┘    │  ennuste: joka 60 min         │  │
│          │           └──────────────┬────────────────┘  │
└──────────┼───────────────────────────┼──────────────────┘
           │                           │ WFS XML -pyynnöt
┌──────────▼──────────┐   ┌───────────▼────────────────┐
│   PostgreSQL 16 +   │   │   opendata.fmi.fi           │
│   TimescaleDB       │   │                             │
│                     │   │  fmi::observations::        │
│  stations           │   │    weather::               │
│  observations       │   │    multipointcoverage      │
│  forecasts          │   │                             │
│  fetch_log          │   │  fmi::forecast::harmonie:: │
│                     │   │    surface::point::        │
│  Hypertaulut +      │   │    multipointcoverage      │
│  automaattinen      │   │                             │
│  pakkaus            │   │  CC BY 4.0 - ei rekist.    │
└─────────────────────┘   └────────────────────────────┘
```

---

## Komponentit

### 1. Backend (FastAPI + Python 3.12)

| Moduuli | Sijainti | Tehtävä |
|---------|----------|---------|
| `main.py` | `app/` | Sovelluksen käynnistys, CORS, lifespan |
| `config.py` | `app/` | Ympäristömuuttujat (pydantic-settings) |
| `database.py` | `app/` | SQLAlchemy async engine |
| `fmi/client.py` | `app/fmi/` | FMI WFS -pyynnöt (fmiopendata) |
| `fmi/parsers.py` | `app/fmi/` | XML-parserointi → Python-dict |
| `scheduler/jobs.py` | `app/scheduler/` | APScheduler-ajastustyöt |
| `services/` | `app/services/` | Tietokantalogiikka |
| `routers/` | `app/routers/` | HTTP-päätepisteet |
| `models/` | `app/models/` | SQLAlchemy ORM -mallit |

### 2. Tietokanta (PostgreSQL 16 + TimescaleDB)

TimescaleDB tarjoaa:
- **Hypertaulut**: automaattinen aikapartitiointi (1 viikko/osio)
- **time_bucket()**: tehokkaat tunti/päivä-aggregaatit
- **Kompressio**: vanhat tiedot pakataan automaattisesti

### 3. Frontend (React 18 + TypeScript + Vite)

| Komponentti | Tehtävä |
|-------------|---------|
| `Dashboard.tsx` | Pääsivu: kortti + välilehdet |
| `WeatherCard.tsx` | Nykyisten olosuhteiden kortti |
| `TemperatureChart.tsx` | Lämpötilakaavio (Recharts LineChart) |
| `PrecipitationChart.tsx` | Sadekaavio (Recharts ComposedChart) |
| `WindChart.tsx` | Tuulikaavio (Recharts Area + Line) |
| `ForecastTable.tsx` | Taulukkoennuste |
| `Header.tsx` | Ylätunniste + asemavalinta |

---

## Teknologiapino

| Taso | Teknologia | Versio | Perustelu |
|------|-----------|--------|-----------|
| Backend | Python | 3.12 | Moderni async-tuki |
| Web-kehys | FastAPI | ≥0.111 | Nopea, async, auto-dokumentaatio |
| ORM | SQLAlchemy | ≥2.0 | Async-tuki, type-safe |
| Tietokanta | PostgreSQL + TimescaleDB | 16 | Aikasarjaoptimoitu |
| Ajastin | APScheduler | ≥3.10 | Sisäinen ajastin, ei Celery-riippuvuutta |
| FMI-asiakas | fmiopendata | ≥0.2 | Valmis WFS-parseri |
| Frontend | React + TypeScript | 18 + 5 | Komponenttimalli, tyyppiturvallisuus |
| Build | Vite | ≥5 | Nopea kehityspalvelin |
| Kaaviot | Recharts | ≥2.12 | React-natiivi, aikasarjatuki |
| Tyyli | Tailwind CSS | ≥3.4 | Utility-first, tumma teema |
| Datan haku | SWR | ≥2.2 | Cache + automaattinen päivitys |
| Kontteja | Docker + Compose | - | Helppo käyttöönotto |

---

## Tietokantarakenne

### `stations` – Asemien metatiedot
```
fmisid INTEGER PK  – FMI-asematunnus
name   TEXT        – Aseman nimi (esim. "Helsinki Kaisaniemi")
region TEXT        – Maakunta
latitude, longitude FLOAT
elevation FLOAT    – Korkeus merenpinnasta (m)
is_active BOOLEAN
```

### `observations` – Havainnot (hypertaulu)
```
time   TIMESTAMPTZ PK  – Havaintohetki (10 min välein)
fmisid INTEGER      PK  – Asematunnus
temperature FLOAT       – Lämpötila (°C)
humidity FLOAT          – Suhteellinen kosteus (%)
wind_speed FLOAT        – Tuulennopeus (m/s)
wind_gust FLOAT         – Tuulenpuuska (m/s)
wind_direction SMALLINT – Tuulensuunta (°)
precipitation_1h FLOAT  – Sademäärä/tunti (mm)
snow_depth FLOAT        – Lumensyvyys (cm)
pressure FLOAT          – Ilmanpaine (hPa)
visibility INTEGER      – Näkyvyys (m)
cloud_cover SMALLINT    – Pilvisyys (oktat 0-8)
```

### `forecasts` – Ennusteet (hypertaulu)
```
id BIGSERIAL PK
fetched_at TIMESTAMPTZ  – Hakuhetki
valid_time TIMESTAMPTZ  – Ennusteen kohdeaika
fmisid INTEGER          – Asematunnus
place_name TEXT         – Paikkakunta
model TEXT              – 'harmonie' | 'ecmwf'
temperature, wind_speed, ... – Ennustemuuttujat
weather_symbol SMALLINT – WMO-sääsymboli
```

### `fetch_log` – Hakuhistoria
```
id BIGSERIAL PK
job_type TEXT    – 'observations' | 'forecast_harmonie'
status TEXT      – 'running' | 'success' | 'error'
records_fetched  – Haettujen rivien määrä
error_message    – Virheviesti (jos epäonnistui)
```

---

## Datavirta

```
1. APScheduler käynnistää haun (joka 10 min)
   ↓
2. FMI WFS -pyyntö (multipointcoverage, bbox=Suomi)
   ↓
3. fmiopendata parsii XML → Python-dict
   ↓
4. parsers.py muuntaa dict → SQLAlchemy-rivit
   ↓
5. UPSERT observations (ON CONFLICT DO NOTHING)
   ↓
6. React-frontend pyytää /api/v1/observations
   ↓
7. SQLAlchemy-kysely → JSON-vastaus
   ↓
8. Recharts piirtää kaaviot
```

---

## Ajastus

| Työ | Väli | Kuvaus |
|-----|------|--------|
| `fetch_observations_job` | 10 min | Koko Suomi, bbox-kysely |
| `fetch_harmonie_job` | 60 min | Pistepisteet kaikille paikkakunnille |
| Käynnistys | Heti | Molemmat ajetaan käynnistyksen yhteydessä |

Ajastin käyttää `max_instances=1` ja `coalesce=True`, joten vain yksi instanssi kerrallaan.
