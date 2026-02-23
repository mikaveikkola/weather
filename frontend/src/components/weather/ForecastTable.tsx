import type { ForecastPoint } from '../../types/forecast'
import { formatDateTime, formatHumidity, formatPrecip, formatTemp, formatWind } from '../../utils/formatters'

interface ForecastTableProps {
  data: ForecastPoint[]
}

export default function ForecastTable({ data }: ForecastTableProps) {
  if (!data.length) {
    return (
      <div className="text-center text-gray-500 py-8">
        Ei ennustetietoja saatavilla
      </div>
    )
  }

  return (
    <div className="overflow-x-auto rounded-xl border border-gray-800">
      <table className="w-full text-sm">
        <thead>
          <tr className="bg-gray-800 text-gray-400 text-xs uppercase tracking-wider">
            <th className="px-4 py-3 text-left">Aika</th>
            <th className="px-4 py-3 text-right">Lämpötila</th>
            <th className="px-4 py-3 text-right">Tuuli</th>
            <th className="px-4 py-3 text-right">Sade</th>
            <th className="px-4 py-3 text-right hidden sm:table-cell">Kosteus</th>
          </tr>
        </thead>
        <tbody>
          {data.map((row, i) => (
            <tr
              key={row.id ?? row.valid_time}
              className={`border-t border-gray-800 ${i % 2 === 0 ? 'bg-gray-900' : 'bg-gray-950'} hover:bg-gray-800 transition-colors`}
            >
              <td className="px-4 py-3 text-gray-300 whitespace-nowrap">
                {formatDateTime(row.valid_time)}
              </td>
              <td className={`px-4 py-3 text-right font-semibold tabular-nums`}>
                {formatTemp(row.temperature)}
              </td>
              <td className="px-4 py-3 text-right text-gray-300 whitespace-nowrap">
                {formatWind(row.wind_speed, row.wind_direction)}
              </td>
              <td className="px-4 py-3 text-right text-cyan-400">
                {formatPrecip(row.precipitation_1h)}
              </td>
              <td className="px-4 py-3 text-right text-blue-300 hidden sm:table-cell">
                {formatHumidity(row.humidity)}
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}
