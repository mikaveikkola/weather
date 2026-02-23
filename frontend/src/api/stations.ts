import type { AxiosInstance } from 'axios'
import type { Station, StationLatest } from '../types/station'

export async function fetchStations(client: AxiosInstance): Promise<Station[]> {
  const { data } = await client.get<Station[]>('/stations')
  return data
}

export async function fetchStation(client: AxiosInstance, fmisid: number): Promise<Station> {
  const { data } = await client.get<Station>(`/stations/${fmisid}`)
  return data
}

export async function fetchLatestObservations(
  client: AxiosInstance,
  fmisids?: number[],
): Promise<StationLatest[]> {
  const params = fmisids?.length ? { fmisids: fmisids.join(',') } : {}
  const { data } = await client.get<StationLatest[]>('/observations/latest', { params })
  return data
}
