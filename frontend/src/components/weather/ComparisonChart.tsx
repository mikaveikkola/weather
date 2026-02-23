import { format, parseISO, startOfHour } from 'date-fns'
import { fi } from 'date-fns/locale'
import {
  Bar,
  CartesianGrid,
  Cell,
  ComposedChart,
  Legend,
  Line,
  ReferenceLine,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from 'recharts'
import type { ForecastPoint } from '../../types/forecast'
import type { ObservationPoint } from '../../types/observation'

// ─── Data matching ────────────────────────────────────────────────────────────

function roundToHour(iso: string): string {
  return startOfHour(parseISO(iso)).toISOString()
}

interface ChartRow {
  time: string
  obsTemp: number | null;  fcTemp: number | null;  diffTemp: number | null
  obsWind: number | null;  fcWind: number | null;  diffWind: number | null
  obsHum:  number | null;  fcHum:  number | null;  diffHum:  number | null
  obsPres: number | null;  fcPres: number | null;  diffPres: number | null
}

function sub(a: number | null, b: number | null): number | null {
  return a != null && b != null ? parseFloat((b - a).toFixed(2)) : null
}

function buildChartData(obs: ObservationPoint[], fc: ForecastPoint[]): ChartRow[] {
  const obsMap = new Map<string, ObservationPoint>()
  for (const o of obs) obsMap.set(roundToHour(o.time), o)

  const rows: ChartRow[] = []
  for (const f of fc) {
    const hour = roundToHour(f.valid_time)
    const o = obsMap.get(hour)
    if (!o) continue
    rows.push({
      time:     format(parseISO(hour), 'dd.MM. HH:mm', { locale: fi }),
      obsTemp:  o.temperature,        fcTemp:  f.temperature,        diffTemp:  sub(o.temperature,  f.temperature),
      obsWind:  o.wind_speed,         fcWind:  f.wind_speed,         diffWind:  sub(o.wind_speed,   f.wind_speed),
      obsHum:   o.humidity,           fcHum:   f.humidity,           diffHum:   sub(o.humidity,     f.humidity),
      obsPres:  o.pressure,           fcPres:  f.pressure,           diffPres:  sub(o.pressure,     f.pressure),
    })
  }
  rows.sort((a, b) => a.time.localeCompare(b.time))
  return rows
}

// ─── Single metric chart ──────────────────────────────────────────────────────

interface MetricChartProps {
  data: ChartRow[]
  title: string
  unit: string
  obsKey: keyof ChartRow
  fcKey: keyof ChartRow
  diffKey: keyof ChartRow
  decimals?: number
}

function MetricTooltip({ active, payload, label, unit, decimals }: any) {
  if (!active || !payload?.length) return null
  const get = (key: string) => payload.find((p: any) => p.dataKey === key)?.value ?? null
  const obs  = get('obs')
  const fc   = get('fc')
  const diff = get('diff')
  const fmt  = (v: number) => v.toFixed(decimals ?? 1)
  const sign = (v: number) => (v > 0 ? '+' : '')
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-3 text-xs shadow-xl space-y-1">
      <p className="text-gray-400 mb-1">{label}</p>
      {obs  != null && <p className="text-blue-400">Havainto: {fmt(obs)}{unit}</p>}
      {fc   != null && <p className="text-orange-400">Ennuste: {fmt(fc)}{unit}</p>}
      {diff != null && (
        <p className={diff >= 0 ? 'text-red-400' : 'text-emerald-400'}>
          Ero: {sign(diff)}{fmt(diff)}{unit}
        </p>
      )}
    </div>
  )
}

function MetricChart({ data, title, unit, obsKey, fcKey, diffKey, decimals = 1 }: MetricChartProps) {
  // Remap to generic obs/fc/diff keys so recharts legend works simply
  const mapped = data.map((d) => ({
    time: d.time,
    obs:  d[obsKey]  as number | null,
    fc:   d[fcKey]   as number | null,
    diff: d[diffKey] as number | null,
  }))

  return (
    <div>
      <h4 className="text-xs font-semibold uppercase tracking-widest text-gray-500 mb-2">{title}</h4>
      <ResponsiveContainer width="100%" height={230}>
        <ComposedChart data={mapped} margin={{ top: 5, right: 55, left: 0, bottom: 5 }}>
          <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
          <XAxis
            dataKey="time"
            tick={{ fill: '#9ca3af', fontSize: 10 }}
            tickLine={false}
            interval="preserveStartEnd"
          />
          {/* Left axis: actual values */}
          <YAxis
            yAxisId="val"
            tick={{ fill: '#9ca3af', fontSize: 10 }}
            tickLine={false}
            axisLine={false}
            width={42}
            tickFormatter={(v) => `${v}${unit}`}
          />
          {/* Right axis: difference */}
          <YAxis
            yAxisId="diff"
            orientation="right"
            tick={{ fill: '#6b7280', fontSize: 10 }}
            tickLine={false}
            axisLine={false}
            width={42}
            tickFormatter={(v) => `${v > 0 ? '+' : ''}${parseFloat(v.toFixed(1))}`}
          />
          <Tooltip
            content={<MetricTooltip unit={unit} decimals={decimals} />}
          />
          <Legend
            wrapperStyle={{ fontSize: 11, color: '#9ca3af' }}
            formatter={(value: string) =>
              value === 'obs' ? 'Havainto' : value === 'fc' ? 'Ennuste' : 'Ero (oik.)'
            }
          />

          {/* Difference bars (background) */}
          <ReferenceLine yAxisId="diff" y={0} stroke="#4b5563" strokeDasharray="2 2" />
          <Bar yAxisId="diff" dataKey="diff" name="diff" barSize={10} opacity={0.65} radius={[2,2,0,0]}>
            {mapped.map((d, i) => (
              <Cell
                key={i}
                fill={d.diff == null ? 'transparent' : d.diff >= 0 ? '#ef4444' : '#10b981'}
              />
            ))}
          </Bar>

          {/* Observation line */}
          <Line
            yAxisId="val"
            type="monotone"
            dataKey="obs"
            name="obs"
            stroke="#60a5fa"
            strokeWidth={2.5}
            dot={false}
            connectNulls
          />
          {/* Forecast line */}
          <Line
            yAxisId="val"
            type="monotone"
            dataKey="fc"
            name="fc"
            stroke="#fb923c"
            strokeWidth={2}
            strokeDasharray="6 3"
            dot={false}
            connectNulls
          />
        </ComposedChart>
      </ResponsiveContainer>
    </div>
  )
}

// ─── Main component ───────────────────────────────────────────────────────────

interface Props {
  observations: ObservationPoint[]
  forecasts: ForecastPoint[]
}

export default function ComparisonChart({ observations, forecasts }: Props) {
  const data = buildChartData(observations, forecasts)

  if (data.length === 0) {
    return (
      <div className="text-center text-gray-500 py-8 text-sm">
        Ei yhteisiä ajanhetkiä graafin piirtämiseen.
      </div>
    )
  }

  return (
    <div className="space-y-8">
      <div className="text-xs text-gray-600 flex gap-6">
        <span><span className="text-blue-400">──</span> Havainto</span>
        <span><span className="text-orange-400">╌╌</span> Ennuste</span>
        <span><span className="text-emerald-400">█</span> Ennuste liian matala</span>
        <span><span className="text-red-400">█</span> Ennuste liian korkea</span>
        <span className="text-gray-500">(pylväät oikealla akselilla)</span>
      </div>

      <MetricChart
        data={data}
        title="Lämpötila"
        unit="°C"
        obsKey="obsTemp"
        fcKey="fcTemp"
        diffKey="diffTemp"
      />
      <MetricChart
        data={data}
        title="Tuulennopeus"
        unit=" m/s"
        obsKey="obsWind"
        fcKey="fcWind"
        diffKey="diffWind"
      />
      <MetricChart
        data={data}
        title="Suhteellinen kosteus"
        unit="%"
        obsKey="obsHum"
        fcKey="fcHum"
        diffKey="diffHum"
        decimals={0}
      />
      <MetricChart
        data={data}
        title="Ilmanpaine"
        unit=" hPa"
        obsKey="obsPres"
        fcKey="fcPres"
        diffKey="diffPres"
      />
    </div>
  )
}
