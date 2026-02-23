export interface ForecastPoint {
  id: number
  valid_time: string
  fetched_at: string
  fmisid: number | null
  place_name: string | null
  model: string
  temperature: number | null
  wind_speed: number | null
  wind_direction: number | null
  wind_gust: number | null
  precipitation_1h: number | null
  humidity: number | null
  pressure: number | null
  cloud_cover: number | null
  dew_point: number | null
  weather_symbol: number | null
}

export interface ForecastResponse {
  station: { fmisid: number; name: string } | null
  model: string
  fetched_at: string | null
  data: ForecastPoint[]
}
