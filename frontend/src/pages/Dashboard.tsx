import { useState } from 'react'
import Header from '../components/layout/Header'
import PrecipitationChart from '../components/charts/PrecipitationChart'
import TemperatureChart from '../components/charts/TemperatureChart'
import WindChart from '../components/charts/WindChart'
import ComparisonTable from '../components/weather/ComparisonTable'
import ForecastTable from '../components/weather/ForecastTable'
import WeatherCard from '../components/weather/WeatherCard'
import { useForecastsByPlace } from '../hooks/useForecasts'
import { useObservations } from '../hooks/useObservations'
import { useLatestObservations, useStations } from '../hooks/useStations'
import { DEFAULT_FMISID, TIME_RANGES } from '../utils/constants'
import { formatTemp } from '../utils/formatters'

type Tab = 'havainnot' | 'ennuste' | 'vertailu'

function SectionTitle({ children }: { children: React.ReactNode }) {
  return (
    <h3 className="text-xs font-semibold uppercase tracking-widest text-gray-500 mb-3">
      {children}
    </h3>
  )
}

function SummaryBadge({ label, value }: { label: string; value: string }) {
  return (
    <div className="bg-gray-800 rounded-lg px-3 py-2 text-center">
      <div className="text-xs text-gray-500 mb-0.5">{label}</div>
      <div className="text-sm font-semibold text-gray-200">{value}</div>
    </div>
  )
}

export default function Dashboard() {
  const [selectedFmisid, setSelectedFmisid] = useState<number>(DEFAULT_FMISID)
  const [selectedHours, setSelectedHours] = useState<number>(24)
  const [activeTab, setActiveTab] = useState<Tab>('havainnot')

  const { data: stations = [], isLoading: stationsLoading } = useStations()
  const { data: latestList = [] } = useLatestObservations()
  const { data: obsData, isLoading: obsLoading } = useObservations(selectedFmisid, selectedHours)
  // Always fetch 48h of observations so the comparison tab has enough data
  const { data: obs48h } = useObservations(selectedFmisid, 48)

  const selectedStation = stations.find((s) => s.fmisid === selectedFmisid)
  const placeName = selectedStation?.name?.split(' ')[0] ?? null
  const { data: forecastData, isLoading: forecastLoading } = useForecastsByPlace(placeName)

  const currentLatest = latestList.find((s) => s.fmisid === selectedFmisid)

  const observations = obsData?.data ?? []
  const observations48h = obs48h?.data ?? []
  const forecasts = forecastData?.data ?? []

  // Summary from chart data
  const temps = observations.map((d) => d.temperature).filter((t) => t != null) as number[]
  const minTemp = temps.length ? Math.min(...temps) : null
  const maxTemp = temps.length ? Math.max(...temps) : null
  const totalPrecip = observations.reduce((sum, d) => sum + (d.precipitation_1h ?? 0), 0)

  if (stationsLoading) {
    return (
      <div className="flex items-center justify-center min-h-screen">
        <div className="text-gray-500 text-lg">Ladataan...</div>
      </div>
    )
  }

  const TABS: { id: Tab; label: string }[] = [
    { id: 'havainnot', label: '📊 Havainnot' },
    { id: 'ennuste',   label: '🔮 Ennuste' },
    { id: 'vertailu',  label: '⚖️ Vertailu' },
  ]

  return (
    <div className="min-h-screen bg-gray-950">
      <Header
        stations={stations}
        selectedFmisid={selectedFmisid}
        onStationChange={setSelectedFmisid}
        lastUpdate={currentLatest?.time}
      />

      <main className="max-w-7xl mx-auto px-4 py-6 space-y-6">
        {/* Current conditions card */}
        {currentLatest ? (
          <WeatherCard data={currentLatest} />
        ) : (
          <div className="bg-gray-900 rounded-2xl border border-gray-800 p-8 text-center text-gray-500">
            Ei havaintotietoja asemalle. Tiedot päivittyvät automaattisesti.
          </div>
        )}

        {/* Summary row */}
        {temps.length > 0 && (
          <div>
            <SectionTitle>Yhteenveto ({TIME_RANGES.find((r) => r.hours === selectedHours)?.label})</SectionTitle>
            <div className="flex flex-wrap gap-2">
              <SummaryBadge label="Min lämpötila" value={formatTemp(minTemp)} />
              <SummaryBadge label="Max lämpötila" value={formatTemp(maxTemp)} />
              <SummaryBadge label="Sade yht." value={`${totalPrecip.toFixed(1)} mm`} />
            </div>
          </div>
        )}

        {/* Tabs */}
        <div>
          <div className="flex gap-1 mb-4 bg-gray-900 p-1 rounded-xl w-fit">
            {TABS.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`px-4 py-2 rounded-lg text-sm font-medium transition-colors ${
                  activeTab === tab.id
                    ? 'bg-blue-600 text-white'
                    : 'text-gray-400 hover:text-gray-200 hover:bg-gray-800'
                }`}
              >
                {tab.label}
              </button>
            ))}
          </div>

          {/* Observations tab */}
          {activeTab === 'havainnot' && (
            <div className="space-y-6">
              <div className="flex gap-2 flex-wrap">
                {TIME_RANGES.map((range) => (
                  <button
                    key={range.hours}
                    onClick={() => setSelectedHours(range.hours)}
                    className={`px-3 py-1.5 rounded-lg text-sm transition-colors ${
                      selectedHours === range.hours
                        ? 'bg-blue-600 text-white'
                        : 'bg-gray-800 text-gray-400 hover:bg-gray-700'
                    }`}
                  >
                    {range.label}
                  </button>
                ))}
              </div>

              {obsLoading ? (
                <div className="text-center text-gray-500 py-12">Ladataan havaintoja...</div>
              ) : observations.length === 0 ? (
                <div className="text-center text-gray-500 py-12">
                  Ei havaintoja valitulle ajanjaksolle
                </div>
              ) : (
                <>
                  <div className="bg-gray-900 rounded-xl border border-gray-800 p-4">
                    <SectionTitle>Lämpötila</SectionTitle>
                    <TemperatureChart data={observations} showDewPoint />
                  </div>
                  <div className="bg-gray-900 rounded-xl border border-gray-800 p-4">
                    <SectionTitle>Sademäärä & lumi</SectionTitle>
                    <PrecipitationChart data={observations} />
                  </div>
                  <div className="bg-gray-900 rounded-xl border border-gray-800 p-4">
                    <SectionTitle>Tuuli</SectionTitle>
                    <WindChart data={observations} />
                  </div>
                </>
              )}
            </div>
          )}

          {/* Forecast tab */}
          {activeTab === 'ennuste' && (
            <div>
              {forecastLoading ? (
                <div className="text-center text-gray-500 py-12">Ladataan ennustetta...</div>
              ) : !forecastData?.data?.length ? (
                <div className="text-center text-gray-500 py-12">
                  Ei ennustetietoja saatavilla
                </div>
              ) : (
                <>
                  {forecastData.fetched_at && (
                    <p className="text-xs text-gray-600 mb-3">
                      Ennuste haettu: {forecastData.fetched_at}
                    </p>
                  )}
                  <ForecastTable data={forecastData.data} />
                </>
              )}
            </div>
          )}

          {/* Comparison tab */}
          {activeTab === 'vertailu' && (
            <div className="bg-gray-900 rounded-xl border border-gray-800 p-4">
              <SectionTitle>Ennuste vs. toteutunut — {placeName ?? selectedStation?.name}</SectionTitle>
              <p className="text-xs text-gray-600 mb-4">
                Vertaa viimeisintä Harmonie-ennustetta toteutuneisiin havaintoihin tunneittain.
                Ero = ennuste – havainto.
              </p>
              <ComparisonTable observations={observations48h} forecasts={forecasts} />
            </div>
          )}
        </div>
      </main>
    </div>
  )
}
