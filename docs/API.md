# API-dokumentaatio – FMI Säätiedot

Kaikki päätepisteet alkavat polulla `/api/v1/`.

Interaktiivinen dokumentaatio: http://localhost:8000/docs

---

## Health / Terveystarkistus

### `GET /api/v1/health`
Palauttaa järjestelmän tilan.

**Vastaus:**
```json
{
  "status": "ok",
  "db_connected": true,
  "last_observation": "2026-02-20T12:50:00+00:00",
  "last_fetch": "2026-02-20T12:55:00+00:00",
  "server_time": "2026-02-20T13:00:00+00:00"
}
```

---

## Asemat / Stations

### `GET /api/v1/stations`
Listaa kaikki asemat.

**Parametrit:**
| Parametri | Tyyppi | Oletus | Kuvaus |
|-----------|--------|--------|--------|
| `active` | bool | `true` | Vain aktiiviset asemat |

**Vastaus:**
```json
[
  {
    "fmisid": 100971,
    "name": "Helsinki Kaisaniemi",
    "region": "Uusimaa",
    "country": "Finland",
    "latitude": 60.1754,
    "longitude": 24.9464,
    "elevation": 4.0,
    "station_type": "weather",
    "is_active": true
  }
]
```

### `GET /api/v1/stations/{fmisid}`
Yksittäisen aseman tiedot.

### `GET /api/v1/stations/{fmisid}/latest`
Aseman viimeisin havainto.

**Vastaus:**
```json
{
  "fmisid": 100971,
  "name": "Helsinki Kaisaniemi",
  "region": "Uusimaa",
  "latitude": 60.1754,
  "longitude": 24.9464,
  "time": "2026-02-20T12:50:00+00:00",
  "temperature": -2.3,
  "humidity": 82.0,
  "wind_speed": 4.1,
  "wind_direction": 220,
  "wind_gust": 6.8,
  "precipitation_1h": 0.0,
  "snow_depth": 5.0,
  "pressure": 1012.4
}
```

---

## Havainnot / Observations

### `GET /api/v1/observations`
Aikasarjadata asemalle.

**Parametrit:**
| Parametri | Tyyppi | Pakollinen | Kuvaus |
|-----------|--------|-----------|--------|
| `fmisid` | int | Kyllä | Asematunnus |
| `start` | datetime | Ei | Alkuaika (ISO 8601, oletus: 24h sitten) |
| `end` | datetime | Ei | Loppuaika (ISO 8601, oletus: nyt) |
| `resolution` | string | Ei | `raw` / `hourly` / `daily` / `auto` |

**Vastaus:**
```json
{
  "station": {
    "fmisid": 100971,
    "name": "Helsinki Kaisaniemi",
    "latitude": 60.1754,
    "longitude": 24.9464
  },
  "resolution": "hourly",
  "data": [
    {
      "time": "2026-02-20T10:00:00+00:00",
      "temperature": -3.2,
      "dew_point": -7.1,
      "humidity": 74.0,
      "wind_speed": 3.5,
      "wind_gust": 5.8,
      "wind_direction": 215,
      "precipitation_1h": 0.0,
      "snow_depth": 5.0,
      "pressure": 1013.2,
      "visibility": 10000,
      "cloud_cover": 7
    }
  ]
}
```

### `GET /api/v1/observations/latest`
Viimeisin havainto per asema.

**Parametrit:**
| Parametri | Tyyppi | Kuvaus |
|-----------|--------|--------|
| `fmisids` | string | Pilkulla erotettu lista (esim. `100971,101004`). Tyhjä = kaikki. |

### `GET /api/v1/observations/summary`
Yhteenveto ajanjaksolle.

**Parametrit:**
| Parametri | Tyyppi | Kuvaus |
|-----------|--------|--------|
| `fmisid` | int | Asematunnus (pakollinen) |
| `period` | string | `24h` / `7d` / `30d` |

**Vastaus:**
```json
{
  "fmisid": 100971,
  "period": "24h",
  "min_temperature": -5.1,
  "max_temperature": -0.2,
  "avg_temperature": -2.8,
  "avg_humidity": 80.5,
  "avg_wind_speed": 3.2,
  "max_wind_gust": 9.4,
  "total_precipitation": 0.6,
  "avg_pressure": 1011.8
}
```

---

## Ennusteet / Forecasts

### `GET /api/v1/forecasts`
Ennusteaikasarja asemalle tai paikkakunnalle.

**Parametrit:**
| Parametri | Tyyppi | Kuvaus |
|-----------|--------|--------|
| `fmisid` | int | Asematunnus |
| `place` | string | Paikkakunta (esim. `Helsinki`) |
| `model` | string | `harmonie` (oletus) |
| `hours` | int | Ennustehorisontti tunteina (max 240) |

*Joko `fmisid` tai `place` on pakollinen.*

**Vastaus:**
```json
{
  "station": null,
  "model": "harmonie",
  "fetched_at": "2026-02-20T12:00:00+00:00",
  "data": [
    {
      "id": 12345,
      "valid_time": "2026-02-20T13:00:00+00:00",
      "fetched_at": "2026-02-20T12:00:00+00:00",
      "fmisid": 100971,
      "place_name": "Helsinki",
      "model": "harmonie",
      "temperature": -1.5,
      "wind_speed": 5.0,
      "wind_direction": 230,
      "wind_gust": 8.2,
      "precipitation_1h": 0.2,
      "humidity": 85.0,
      "pressure": 1010.5,
      "cloud_cover": 95.0,
      "dew_point": -3.4,
      "weather_symbol": 31
    }
  ]
}
```

---

## Ajastustyöt / Jobs

### `GET /api/v1/jobs/status`
Listaa ajastetut työt ja niiden seuraavan ajoajan.

**Vastaus:**
```json
[
  {
    "id": "obs_fetch",
    "name": "fetch_observations_job",
    "next_run": "2026-02-20T13:10:00+00:00",
    "trigger": "interval[0:10:00]"
  },
  {
    "id": "fc_harmonie",
    "name": "fetch_harmonie_job",
    "next_run": "2026-02-20T13:00:00+00:00",
    "trigger": "interval[1:00:00]"
  }
]
```

### `POST /api/v1/jobs/trigger`
Käynnistää työn manuaalisesti.

**Pyyntö:**
```json
{ "job": "observations" }
```
tai
```json
{ "job": "forecast_harmonie" }
```

**Vastaus:**
```json
{ "status": "triggered", "job": "observations" }
```

### `GET /api/v1/jobs/log`
Hakuhistoria.

**Parametrit:**
| Parametri | Tyyppi | Kuvaus |
|-----------|--------|--------|
| `job_type` | string | Suodatus: `observations` / `forecast_harmonie` |
| `limit` | int | Rivimäärä (max 500, oletus 50) |

**Vastaus:**
```json
[
  {
    "id": 42,
    "job_type": "observations",
    "status": "success",
    "started_at": "2026-02-20T12:50:01+00:00",
    "finished_at": "2026-02-20T12:50:08+00:00",
    "records_fetched": 2847,
    "error_message": null
  }
]
```

---

## HTTP-virheet

| Koodi | Kuvaus |
|-------|--------|
| 400 | Virheellinen pyyntöparametri |
| 404 | Asemaa ei löydy |
| 500 | Palvelinvirhe (tarkista lokit) |
