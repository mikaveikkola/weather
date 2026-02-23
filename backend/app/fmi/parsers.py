import logging
from datetime import datetime, timezone
from typing import Any, Dict, List, Optional

from . import stored_queries as sq

logger = logging.getLogger(__name__)


def _safe_float(val) -> Optional[float]:
    if val is None:
        return None
    try:
        f = float(val)
        if f != f:  # NaN check
            return None
        return f
    except (ValueError, TypeError):
        return None


def _safe_int(val) -> Optional[int]:
    f = _safe_float(val)
    return int(round(f)) if f is not None else None


def _ensure_utc(dt: datetime) -> datetime:
    if dt.tzinfo is None:
        return dt.replace(tzinfo=timezone.utc)
    return dt


def parse_stations(location_metadata: Dict[str, Any]) -> List[dict]:
    """Parse station metadata from fmiopendata location_metadata dict."""
    stations = []
    for name, meta in location_metadata.items():
        try:
            fmisid = int(meta.get("fmisid", 0))
            if not fmisid:
                continue
            stations.append(
                {
                    "fmisid": fmisid,
                    "name": name,
                    "region": meta.get("region"),
                    "country": meta.get("country", "Finland"),
                    "latitude": float(meta.get("latitude", 0)),
                    "longitude": float(meta.get("longitude", 0)),
                    "elevation": _safe_float(meta.get("elevation")),
                    "station_type": "weather",
                    "is_active": True,
                }
            )
        except (ValueError, TypeError) as e:
            logger.warning(f"Could not parse station {name}: {e}")
    return stations


def parse_observations(obs_data: Dict, location_metadata: Dict[str, Any]) -> List[dict]:
    """Parse fmiopendata multipointcoverage observations into list of row dicts."""
    fmisid_map: Dict[str, int] = {}
    for name, meta in location_metadata.items():
        try:
            fid = int(meta.get("fmisid", 0))
            if fid:
                fmisid_map[name] = fid
        except (ValueError, TypeError):
            pass

    rows = []
    for timestamp, stations in obs_data.items():
        ts = _ensure_utc(timestamp)
        for station_name, params in stations.items():
            fmisid = fmisid_map.get(station_name)
            if not fmisid:
                continue
            row: dict = {"time": ts, "fmisid": fmisid}
            for long_name, field in sq.OBS_PARAM_MAP.items():
                param_data = params.get(long_name)
                val = param_data.get("value") if param_data else None
                if field in ("wind_direction", "cloud_cover", "weather_code", "visibility"):
                    row[field] = _safe_int(val)
                else:
                    row[field] = _safe_float(val)
            rows.append(row)
    return rows


def parse_forecast_harmonie(
    fc_data: Dict,
    location_metadata: Dict[str, Any],
    fetched_at: datetime,
    place: str,
) -> List[dict]:
    """Parse Harmonie forecast data into list of row dicts."""
    fmisid: Optional[int] = None
    lat: Optional[float] = None
    lon: Optional[float] = None
    for meta in location_metadata.values():
        fmisid = _safe_int(meta.get("fmisid"))
        lat = _safe_float(meta.get("latitude"))
        lon = _safe_float(meta.get("longitude"))
        break

    rows = []
    for valid_time, locations in fc_data.items():
        vt = _ensure_utc(valid_time)
        for _loc_name, params in locations.items():
            row: dict = {
                "fetched_at": fetched_at,
                "valid_time": vt,
                "fmisid": fmisid,
                "place_name": place,
                "latitude": lat,
                "longitude": lon,
                "model": "harmonie",
            }
            for long_name, field in sq.FORECAST_PARAM_MAP.items():
                param_data = params.get(long_name)
                val = param_data.get("value") if param_data else None
                if field in ("wind_direction", "weather_symbol"):
                    row[field] = _safe_int(val)
                else:
                    row[field] = _safe_float(val)
            rows.append(row)
            break  # point forecast: only one location per timestamp
    return rows
