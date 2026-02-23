import {
  Area,
  CartesianGrid,
  ComposedChart,
  Legend,
  Line,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from 'recharts'
import type { ObservationPoint } from '../../types/observation'
import { formatDateTime } from '../../utils/formatters'

interface WindChartProps {
  data: ObservationPoint[]
}

function CustomTooltip({ active, payload, label }: any) {
  if (!active || !payload?.length) return null
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-3 text-sm shadow-xl">
      <p className="text-gray-400 mb-1">{label}</p>
      {payload.map((p: any) => (
        <p key={p.name} style={{ color: p.color }}>
          {p.name}: {p.value != null ? `${Number(p.value).toFixed(1)} m/s` : '–'}
        </p>
      ))}
    </div>
  )
}

export default function WindChart({ data }: WindChartProps) {
  const chartData = data.map((d) => ({
    time: formatDateTime(d.time),
    wind_speed: d.wind_speed != null ? Number(d.wind_speed.toFixed(1)) : null,
    wind_gust: d.wind_gust != null ? Number(d.wind_gust.toFixed(1)) : null,
  }))

  return (
    <ResponsiveContainer width="100%" height={220}>
      <ComposedChart data={chartData} margin={{ top: 5, right: 10, left: 0, bottom: 5 }}>
        <CartesianGrid strokeDasharray="3 3" stroke="#374151" />
        <XAxis
          dataKey="time"
          tick={{ fill: '#9ca3af', fontSize: 11 }}
          tickLine={false}
          interval="preserveStartEnd"
        />
        <YAxis
          tick={{ fill: '#9ca3af', fontSize: 11 }}
          tickLine={false}
          axisLine={false}
          tickFormatter={(v) => `${v}`}
          unit=" m/s"
          width={48}
        />
        <Tooltip content={<CustomTooltip />} />
        <Legend wrapperStyle={{ color: '#9ca3af', fontSize: 12 }} />
        <Area
          type="monotone"
          dataKey="wind_speed"
          name="Tuulennopeus"
          fill="#1e40af"
          stroke="#3b82f6"
          strokeWidth={2}
          fillOpacity={0.3}
          dot={false}
          connectNulls
        />
        <Line
          type="monotone"
          dataKey="wind_gust"
          name="Puuska"
          stroke="#fb923c"
          strokeWidth={1.5}
          strokeDasharray="5 3"
          dot={false}
          connectNulls
        />
      </ComposedChart>
    </ResponsiveContainer>
  )
}
