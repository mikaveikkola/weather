export interface Station {
  fmisid: number
  name: string
  region: string | null
  country: string
  latitude: number
  longitude: number
  elevation: number | null
  station_type: string | null
  is_active: boolean
}

export interface StationLatest {
  fmisid: number
  name: string
  region: string | null
  latitude: number
  longitude: number
  time: string | null
  temperature: number | null
  humidity: number | null
  wind_speed: number | null
  wind_direction: number | null
  wind_gust: number | null
  precipitation_1h: number | null
  snow_depth: number | null
  pressure: number | null
}
