import { useMemo } from 'react'
import useSWR from 'swr'
import { createClient } from '../api/client'
import { fetchForecast, fetchForecastByPlace } from '../api/forecasts'
import { useBackend } from '../contexts/BackendContext'

export function useForecasts(fmisid: number | null, model = 'harmonie', hours = 48) {
  const { baseUrl } = useBackend()
  const client = useMemo(() => createClient(baseUrl), [baseUrl])
  return useSWR(
    fmisid ? [baseUrl, 'forecast', fmisid, model, hours] : null,
    () => fetchForecast(client, fmisid!, model, hours),
    { refreshInterval: 3_600_000 },
  )
}

export function useForecastsByPlace(place: string | null, model = 'harmonie') {
  const { baseUrl } = useBackend()
  const client = useMemo(() => createClient(baseUrl), [baseUrl])
  return useSWR(
    place ? [baseUrl, 'forecast-place', place, model] : null,
    () => fetchForecastByPlace(client, place!, model),
    { refreshInterval: 3_600_000 },
  )
}
