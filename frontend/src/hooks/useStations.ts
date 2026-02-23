import { useMemo } from 'react'
import useSWR from 'swr'
import { createClient } from '../api/client'
import { fetchLatestObservations, fetchStations } from '../api/stations'
import { useBackend } from '../contexts/BackendContext'

export function useStations() {
  const { baseUrl } = useBackend()
  const client = useMemo(() => createClient(baseUrl), [baseUrl])
  return useSWR([baseUrl, 'stations'], () => fetchStations(client), { refreshInterval: 300_000 })
}

export function useLatestObservations(fmisids?: number[]) {
  const { baseUrl } = useBackend()
  const client = useMemo(() => createClient(baseUrl), [baseUrl])
  const key = fmisids?.length
    ? [baseUrl, 'latest', fmisids.join(',')]
    : [baseUrl, 'latest-all']
  return useSWR(key, () => fetchLatestObservations(client, fmisids), { refreshInterval: 60_000 })
}
