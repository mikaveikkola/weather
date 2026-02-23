import type { AxiosInstance } from 'axios'
import type { ForecastResponse } from '../types/forecast'

export async function fetchForecast(
  client: AxiosInstance,
  fmisid: number,
  model = 'harmonie',
  hours = 48,
): Promise<ForecastResponse> {
  const { data } = await client.get<ForecastResponse>('/forecasts', {
    params: { fmisid, model, hours },
  })
  return data
}

export async function fetchForecastByPlace(
  client: AxiosInstance,
  place: string,
  model = 'harmonie',
): Promise<ForecastResponse> {
  const { data } = await client.get<ForecastResponse>('/forecasts', {
    params: { place, model },
  })
  return data
}

export async function fetchForecastHistory(
  client: AxiosInstance,
  place: string,
  model = 'harmonie',
  hours = 48,
): Promise<ForecastResponse> {
  const { data } = await client.get<ForecastResponse>('/forecasts/history', {
    params: { place, model, hours },
  })
  return data
}
