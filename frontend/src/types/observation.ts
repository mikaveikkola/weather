export interface ObservationPoint {
  time: string
  temperature: number | null
  dew_point: number | null
  humidity: number | null
  wind_speed: number | null
  wind_gust: number | null
  wind_direction: number | null
  precipitation_1h: number | null
  snow_depth: number | null
  pressure: number | null
  visibility: number | null
  cloud_cover: number | null
}

export interface ObservationResponse {
  station: {
    fmisid: number
    name: string
    latitude: number
    longitude: number
  }
  resolution: string
  data: ObservationPoint[]
}

export interface ObservationSummary {
  min_temperature: number | null
  max_temperature: number | null
  avg_temperature: number | null
  avg_humidity: number | null
  avg_wind_speed: number | null
  max_wind_gust: number | null
  total_precipitation: number | null
  avg_pressure: number | null
}
