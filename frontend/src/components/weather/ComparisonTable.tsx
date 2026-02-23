import type { ObservationPoint } from '../../types/observation'
import type { ForecastPoint } from '../../types/forecast'
import { format, parseISO, startOfHour } from 'date-fns'
import { fi } from 'date-fns/locale'

interface MatchedRow {
  hour: string       // ISO rounded to hour
  obs: ObservationPoint
  fc: ForecastPoint
}

function roundToHour(iso: string): string {
  return startOfHour(parseISO(iso)).toISOString()
}

function buildRows(obs: ObservationPoint[], fc: ForecastPoint[]): MatchedRow[] {
  const obsMap = new Map<string, ObservationPoint>()
  for (const o of obs) obsMap.set(roundToHour(o.time), o)

  const rows: MatchedRow[] = []
  for (const f of fc) {
    const hour = roundToHour(f.valid_time)
    const o = obsMap.get(hour)
    if (o) rows.push({ hour, obs: o, fc: f })
  }
  rows.sort((a, b) => a.hour.localeCompare(b.hour))
  return rows
}

function diffColor(diff: number | null, warn: number, bad: number): string {
  if (diff === null) return 'text-gray-600'
  const abs = Math.abs(diff)
  if (abs < warn) return 'text-emerald-400'
  if (abs < bad) return 'text-yellow-400'
  return 'text-red-400'
}

function fmt(v: number | null, decimals = 1): string {
  return v != null ? v.toFixed(decimals) : '–'
}

function diff(a: number | null, b: number | null): number | null {
  return a != null && b != null ? b - a : null
}

function DiffCell({ value, warn, bad, unit = '' }: { value: number | null; warn: number; bad: number; unit?: string }) {
  const color = diffColor(value, warn, bad)
  const sign = value != null && value > 0 ? '+' : ''
  return (
    <td className={`px-3 py-2 text-center text-xs font-semibold ${color}`}>
      {value != null ? `${sign}${value.toFixed(1)}${unit}` : '–'}
    </td>
  )
}

function DataCell({ children }: { children: React.ReactNode }) {
  return <td className="px-3 py-2 text-center text-gray-300 text-xs">{children}</td>
}

interface Props {
  observations: ObservationPoint[]
  forecasts: ForecastPoint[]
}

export default function ComparisonTable({ observations, forecasts }: Props) {
  const rows = buildRows(observations, forecasts)

  if (rows.length === 0) {
    return (
      <div className="text-center text-gray-500 py-12">
        <p>Ei yhteisiä ajanhetkiä havainnoille ja ennusteelle.</p>
        <p className="text-xs mt-2 text-gray-600">
          Vertailu onnistuu kun ennuste kattaa jo toteutuneita tunteja.
          Yleensä 2–6 tunnin takautuma on saatavilla.
        </p>
      </div>
    )
  }

  return (
    <div className="overflow-x-auto">
      <table className="w-full text-sm border-collapse">
        <thead>
          <tr className="border-b border-gray-800">
            <th className="px-3 py-2 text-left text-xs font-semibold text-gray-500 uppercase tracking-wider" rowSpan={2}>
              Aika
            </th>
            {/* Temperature */}
            <th className="px-3 py-1 text-center text-xs text-gray-500 uppercase tracking-wider border-l border-gray-800" colSpan={3}>
              Lämpötila (°C)
            </th>
            {/* Wind */}
            <th className="px-3 py-1 text-center text-xs text-gray-500 uppercase tracking-wider border-l border-gray-800" colSpan={3}>
              Tuuli (m/s)
            </th>
            {/* Humidity */}
            <th className="px-3 py-1 text-center text-xs text-gray-500 uppercase tracking-wider border-l border-gray-800" colSpan={3}>
              Kosteus (%)
            </th>
            {/* Pressure */}
            <th className="px-3 py-1 text-center text-xs text-gray-500 uppercase tracking-wider border-l border-gray-800" colSpan={3}>
              Paine (hPa)
            </th>
          </tr>
          <tr className="border-b border-gray-800">
            {['Hav.', 'Enn.', 'Ero'].map((h, i) => (
              <th key={`t${i}`} className={`px-3 py-1 text-center text-xs text-gray-600 ${i === 0 ? 'border-l border-gray-800' : ''}`}>{h}</th>
            ))}
            {['Hav.', 'Enn.', 'Ero'].map((h, i) => (
              <th key={`w${i}`} className={`px-3 py-1 text-center text-xs text-gray-600 ${i === 0 ? 'border-l border-gray-800' : ''}`}>{h}</th>
            ))}
            {['Hav.', 'Enn.', 'Ero'].map((h, i) => (
              <th key={`h${i}`} className={`px-3 py-1 text-center text-xs text-gray-600 ${i === 0 ? 'border-l border-gray-800' : ''}`}>{h}</th>
            ))}
            {['Hav.', 'Enn.', 'Ero'].map((h, i) => (
              <th key={`p${i}`} className={`px-3 py-1 text-center text-xs text-gray-600 ${i === 0 ? 'border-l border-gray-800' : ''}`}>{h}</th>
            ))}
          </tr>
        </thead>
        <tbody>
          {rows.map((row) => {
            const tempDiff  = diff(row.obs.temperature,   row.fc.temperature)
            const windDiff  = diff(row.obs.wind_speed,    row.fc.wind_speed)
            const humDiff   = diff(row.obs.humidity,      row.fc.humidity)
            const presDiff  = diff(row.obs.pressure,      row.fc.pressure)

            return (
              <tr key={row.hour} className="border-b border-gray-800/50 hover:bg-gray-800/30 transition-colors">
                <td className="px-3 py-2 text-xs text-gray-400 whitespace-nowrap">
                  {format(parseISO(row.hour), 'dd.MM. HH:mm', { locale: fi })}
                </td>

                {/* Temperature */}
                <DataCell>{fmt(row.obs.temperature)}</DataCell>
                <DataCell>{fmt(row.fc.temperature)}</DataCell>
                <DiffCell value={tempDiff} warn={1} bad={3} unit="°" />

                {/* Wind */}
                <DataCell>{fmt(row.obs.wind_speed)}</DataCell>
                <DataCell>{fmt(row.fc.wind_speed)}</DataCell>
                <DiffCell value={windDiff} warn={1} bad={3} />

                {/* Humidity */}
                <DataCell>{fmt(row.obs.humidity, 0)}</DataCell>
                <DataCell>{fmt(row.fc.humidity, 0)}</DataCell>
                <DiffCell value={humDiff} warn={5} bad={15} />

                {/* Pressure */}
                <DataCell>{fmt(row.obs.pressure, 1)}</DataCell>
                <DataCell>{fmt(row.fc.pressure, 1)}</DataCell>
                <DiffCell value={presDiff} warn={1} bad={3} />
              </tr>
            )
          })}
        </tbody>
      </table>

      {/* Legend */}
      <div className="flex gap-4 mt-4 px-3 text-xs text-gray-600">
        <span>Eron väri:</span>
        <span className="text-emerald-400">■ hyvä</span>
        <span className="text-yellow-400">■ kohtalainen</span>
        <span className="text-red-400">■ suuri</span>
        <span className="text-gray-600 ml-2">+ = ennuste liian korkea, – = liian matala</span>
      </div>
    </div>
  )
}
