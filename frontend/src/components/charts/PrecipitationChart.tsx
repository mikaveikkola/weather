import {
  Bar,
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

interface PrecipitationChartProps {
  data: ObservationPoint[]
}

function CustomTooltip({ active, payload, label }: any) {
  if (!active || !payload?.length) return null
  return (
    <div className="bg-gray-800 border border-gray-700 rounded-lg p-3 text-sm shadow-xl">
      <p className="text-gray-400 mb-1">{label}</p>
      {payload.map((p: any) => (
        <p key={p.name} style={{ color: p.color }}>
          {p.name}: {p.value != null ? `${Number(p.value).toFixed(1)} ${p.name === 'Lumensyvyys' ? 'cm' : 'mm'}` : '–'}
        </p>
      ))}
    </div>
  )
}

export default function PrecipitationChart({ data }: PrecipitationChartProps) {
  const chartData = data.map((d) => ({
    time: formatDateTime(d.time),
    precipitation_1h: d.precipitation_1h != null ? Number(d.precipitation_1h.toFixed(1)) : null,
    snow_depth: d.snow_depth != null ? Number(d.snow_depth.toFixed(0)) : null,
  }))

  const hasSnow = data.some((d) => d.snow_depth != null && d.snow_depth > 0)

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
          yAxisId="precip"
          tick={{ fill: '#9ca3af', fontSize: 11 }}
          tickLine={false}
          axisLine={false}
          tickFormatter={(v) => `${v}mm`}
          width={40}
        />
        {hasSnow && (
          <YAxis
            yAxisId="snow"
            orientation="right"
            tick={{ fill: '#bfdbfe', fontSize: 11 }}
            tickLine={false}
            axisLine={false}
            tickFormatter={(v) => `${v}cm`}
            width={40}
          />
        )}
        <Tooltip content={<CustomTooltip />} />
        <Legend wrapperStyle={{ color: '#9ca3af', fontSize: 12 }} />
        <Bar
          yAxisId="precip"
          dataKey="precipitation_1h"
          name="Sade (1h)"
          fill="#0ea5e9"
          opacity={0.8}
          maxBarSize={20}
        />
        {hasSnow && (
          <Line
            yAxisId="snow"
            type="monotone"
            dataKey="snow_depth"
            name="Lumensyvyys"
            stroke="#bfdbfe"
            strokeWidth={2}
            dot={false}
            connectNulls
          />
        )}
      </ComposedChart>
    </ResponsiveContainer>
  )
}
