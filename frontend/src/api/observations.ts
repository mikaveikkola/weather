import type { AxiosInstance } from 'axios'
import type { ObservationResponse, ObservationSummary } from '../types/observation'

export async function fetchObservations(
  client: AxiosInstance,
  fmisid: number,
  startIso: string,
  endIso: string,
  resolution = 'auto',
): Promise<ObservationResponse> {
  const { data } = await client.get<ObservationResponse>('/observations', {
    params: { fmisid, start: startIso, end: endIso, resolution },
  })
  return data
}

export async function fetchSummary(
  client: AxiosInstance,
  fmisid: number,
  period = '24h',
): Promise<ObservationSummary> {
  const { data } = await client.get<ObservationSummary>('/observations/summary', {
    params: { fmisid, period },
  })
  return data
}
