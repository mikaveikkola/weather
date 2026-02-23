import type { Station } from '../../types/station'
import { useBackend } from '../../contexts/BackendContext'
import { formatDateTime } from '../../utils/formatters'

interface HeaderProps {
  stations: Station[]
  selectedFmisid: number
  onStationChange: (fmisid: number) => void
  lastUpdate?: string | null
}

export default function Header({ stations, selectedFmisid, onStationChange, lastUpdate }: HeaderProps) {
  const { backend, setBackend } = useBackend()

  return (
    <header className="bg-gray-900 border-b border-gray-800 px-4 py-3">
      <div className="max-w-7xl mx-auto flex flex-wrap items-center justify-between gap-3">
        <div className="flex items-center gap-2">
          <span className="text-2xl">🌦️</span>
          <h1 className="text-xl font-bold text-white">FMI Säätiedot</h1>
        </div>

        <div className="flex items-center gap-4">
          {lastUpdate && (
            <span className="text-xs text-gray-500 hidden sm:block">
              Päivitetty: {formatDateTime(lastUpdate)}
            </span>
          )}

          {/* Backend selector */}
          <div className="flex items-center gap-1 bg-gray-800 border border-gray-700 rounded-lg p-1">
            <button
              onClick={() => setBackend('python')}
              className={`px-3 py-1 text-xs font-medium rounded-md transition-colors ${
                backend === 'python'
                  ? 'bg-blue-600 text-white'
                  : 'text-gray-400 hover:text-gray-200'
              }`}
            >
              Python
            </button>
            <button
              onClick={() => setBackend('cpp')}
              className={`px-3 py-1 text-xs font-medium rounded-md transition-colors ${
                backend === 'cpp'
                  ? 'bg-emerald-600 text-white'
                  : 'text-gray-400 hover:text-gray-200'
              }`}
            >
              C++
            </button>
          </div>

          <select
            value={selectedFmisid}
            onChange={(e) => onStationChange(Number(e.target.value))}
            className="bg-gray-800 border border-gray-700 text-gray-100 text-sm rounded-lg px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500 min-w-48"
          >
            {stations.map((s) => (
              <option key={s.fmisid} value={s.fmisid}>
                {s.name}{s.region ? ` (${s.region})` : ''}
              </option>
            ))}
          </select>
        </div>
      </div>
    </header>
  )
}
