import { subHours } from 'date-fns'
import { useMemo } from 'react'
import useSWR from 'swr'
import { createClient } from '../api/client'
import { fetchObservations, fetchSummary } from '../api/observations'
import { useBackend } from '../contexts/BackendContext'

export function useObservations(fmisid: number | null, hours: number, resolution = 'auto') {
  const { baseUrl } = useBackend()
  const client = useMemo(() => createClient(baseUrl), [baseUrl])
  return useSWR(
    fmisid ? [baseUrl, 'obs', fmisid, hours, resolution] : null,
    () => {
      const end = new Date()
      const start = subHours(end, hours)
      return fetchObservations(client, fmisid!, start.toISOString(), end.toISOString(), resolution)
    },
    { refreshInterval: 300_000 },
  )
}

export function useSummary(fmisid: number | null, period: string) {
  const { baseUrl } = useBackend()
  const client = useMemo(() => createClient(baseUrl), [baseUrl])
  return useSWR(
    fmisid ? [baseUrl, 'summary', fmisid, period] : null,
    () => fetchSummary(client, fmisid!, period),
    { refreshInterval: 300_000 },
  )
}
