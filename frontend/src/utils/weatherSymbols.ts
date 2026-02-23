interface WeatherSymbol {
  emoji: string
  label: string
}

const SYMBOLS: Record<number, WeatherSymbol> = {
  1: { emoji: '☀️', label: 'Selkeää' },
  2: { emoji: '🌤️', label: 'Puolipilvistä' },
  3: { emoji: '☁️', label: 'Pilvistä' },
  21: { emoji: '🌦️', label: 'Kevyttä sadetta' },
  22: { emoji: '🌧️', label: 'Sadetta' },
  23: { emoji: '🌧️', label: 'Runsasta sadetta' },
  31: { emoji: '🌨️', label: 'Kevyttä lumisadetta' },
  32: { emoji: '❄️', label: 'Lumisadetta' },
  33: { emoji: '❄️', label: 'Runsasta lumisadetta' },
  41: { emoji: '⛈️', label: 'Ukkosta' },
  42: { emoji: '⛈️', label: 'Kovaa ukkosta' },
  51: { emoji: '🌫️', label: 'Sumua' },
  61: { emoji: '🌨️', label: 'Räntää' },
  62: { emoji: '🌨️', label: 'Kovaa räntää' },
  71: { emoji: '🌩️', label: 'Sadekuuroja' },
  72: { emoji: '🌩️', label: 'Lumikuuroja' },
}

export function getWeatherSymbol(code: number | null | undefined): WeatherSymbol {
  if (code == null) return { emoji: '❓', label: 'Ei tietoa' }
  return SYMBOLS[code] ?? { emoji: '🌡️', label: `Koodi ${code}` }
}
