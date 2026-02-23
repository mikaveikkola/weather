import { getWeatherSymbol } from '../../utils/weatherSymbols'

interface WeatherIconProps {
  code: number | null | undefined
  showLabel?: boolean
  className?: string
}

export default function WeatherIcon({ code, showLabel = false, className = '' }: WeatherIconProps) {
  const sym = getWeatherSymbol(code)
  return (
    <span className={`inline-flex items-center gap-1 ${className}`}>
      <span className="text-2xl leading-none">{sym.emoji}</span>
      {showLabel && <span className="text-sm text-gray-400">{sym.label}</span>}
    </span>
  )
}
