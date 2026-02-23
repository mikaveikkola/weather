import {
  CartesianGrid,
  Legend,
  Line,
  LineChart,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from 'recharts'
import type { ObservationPoint } from '../../types/observation'
import { formatDateTime, formatTemp } from '../../utils/formatters'

interface TemperatureChartProps {
  data: ObservationPoint[]
  showDewPoint?: boolean
}

function CustomTooltip({ active, payload, label }: any) {
  if (!active || !payload?.length) return null
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-3 text-sm shadow-xl">
      <p className="text-gray-400 mb-1">{label}</p>
      {payload.map((p: any) => (
        <p key={p.name} style={{ color: p.color }}>
          {p.name}: {formatTemp(p.value)}
        </p>
      ))}
    </div>
  )
}

export default function TemperatureChart({ data, showDewPoint = false }: TemperatureChartProps) {
  const chartData = data.map((d) => ({
    ...d,
    time: formatDateTime(d.time),
    temperature: d.temperature != null ? Number(d.temperature.toFixed(1)) : null,
    dew_point: d.dew_point != null ? Number(d.dew_point.toFixed(1)) : null,
  }))

  return (
    <ResponsiveContainer width="100%" height={260}>
      <LineChart data={chartData} margin={{ top: 5, right: 10, left: 0, bottom: 5 }}>
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
          tickFormatter={(v) => `${v > 0 ? '+' : ''}${v}°`}
          width={40}
        />
        <Tooltip content={<CustomTooltip />} />
        <Legend wrapperStyle={{ color: '#9ca3af', fontSize: 12 }} />
        <Line
          type="monotone"
          dataKey="temperature"
          name="Lämpötila"
          stroke="#60a5fa"
          strokeWidth={2}
          dot={false}
          connectNulls
        />
        {showDewPoint && (
          <Line
            type="monotone"
            dataKey="dew_point"
            name="Kastepiste"
            stroke="#93c5fd"
            strokeWidth={1.5}
            strokeDasharray="4 4"
            dot={false}
            connectNulls
          />
        )}
      </LineChart>
    </ResponsiveContainer>
  )
}
