import type { StationLatest } from '../../types/station'
import {
  formatDateTime,
  formatHumidity,
  formatPrecip,
  formatPressure,
  formatSnow,
  formatTemp,
  formatVisibility,
  formatWind,
  tempColor,
} from '../../utils/formatters'

interface WeatherCardProps {
  data: StationLatest
}

function Metric({ icon, label, value, colorClass }: { icon: string; label: string; value: string; colorClass?: string }) {
  return (
    <div className="flex flex-col items-center bg-gray-800 rounded-xl p-3 min-w-24">
      <span className="text-xl mb-1">{icon}</span>
      <span className={`text-sm font-semibold ${colorClass ?? 'text-gray-100'}`}>{value}</span>
      <span className="text-xs text-gray-500 mt-0.5">{label}</span>
    </div>
  )
}

export default function WeatherCard({ data }: WeatherCardProps) {
  return (
    <div className="bg-gray-900 rounded-2xl border border-gray-800 p-5">
      <div className="flex items-start justify-between mb-4">
        <div>
          <h2 className="text-lg font-bold text-white">{data.name}</h2>
          {data.region && <p className="text-sm text-gray-500">{data.region}</p>}
        </div>
        {data.time && (
          <span className="text-xs text-gray-600">{formatDateTime(data.time)}</span>
        )}
      </div>

      {/* Main temperature display */}
      <div className="text-center mb-5">
        <span className={`text-6xl font-bold tabular-nums ${tempColor(data.temperature)}`}>
          {formatTemp(data.temperature)}
        </span>
      </div>

      {/* Metrics grid */}
      <div className="flex flex-wrap gap-2 justify-center">
        <Metric icon="💨" label="Tuuli" value={formatWind(data.wind_speed, data.wind_direction)} />
        <Metric icon="💨" label="Puuska" value={formatWind(data.wind_gust)} />
        <Metric icon="💧" label="Kosteus" value={formatHumidity(data.humidity)} colorClass="text-blue-300" />
        <Metric icon="🌡️" label="Paine" value={formatPressure(data.pressure)} colorClass="text-purple-300" />
        <Metric icon="🌧️" label="Sade (1h)" value={formatPrecip(data.precipitation_1h)} colorClass="text-cyan-300" />
        {data.snow_depth != null && data.snow_depth > 0 && (
          <Metric icon="❄️" label="Lumi" value={formatSnow(data.snow_depth)} colorClass="text-blue-200" />
        )}
      </div>
    </div>
  )
}
