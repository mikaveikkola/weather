import { format, parseISO } from 'date-fns'
import { fi } from 'date-fns/locale'

export function formatTemp(v: number | null | undefined): string {
  if (v == null) return '–'
  return `${v > 0 ? '+' : ''}${v.toFixed(1)} °C`
}

export function formatWind(speed: number | null | undefined, dir?: number | null): string {
  if (speed == null) return '–'
  const dirStr = dir != null ? ` ${windDirToArrow(dir)}` : ''
  return `${speed.toFixed(1)} m/s${dirStr}`
}

export function formatHumidity(v: number | null | undefined): string {
  if (v == null) return '–'
  return `${Math.round(v)} %`
}

export function formatPressure(v: number | null | undefined): string {
  if (v == null) return '–'
  return `${v.toFixed(1)} hPa`
}

export function formatPrecip(v: number | null | undefined): string {
  if (v == null) return '–'
  return `${v.toFixed(1)} mm`
}

export function formatSnow(v: number | null | undefined): string {
  if (v == null) return '–'
  return `${Math.round(v)} cm`
}

export function formatVisibility(v: number | null | undefined): string {
  if (v == null) return '–'
  if (v >= 1000) return `${(v / 1000).toFixed(1)} km`
  return `${v} m`
}

export function formatDateTime(iso: string): string {
  return format(parseISO(iso), 'dd.MM. HH:mm', { locale: fi })
}

export function formatDate(iso: string): string {
  return format(parseISO(iso), 'dd.MM.yyyy', { locale: fi })
}

export function formatTime(iso: string): string {
  return format(parseISO(iso), 'HH:mm', { locale: fi })
}

export function windDirToArrow(deg: number): string {
  const dirs = ['↓', '↙', '←', '↖', '↑', '↗', '→', '↘']
  const idx = Math.round(((deg + 22.5) % 360) / 45) % 8
  return dirs[idx]
}

export function windDirToText(deg: number | null | undefined): string {
  if (deg == null) return '–'
  const dirs = ['P', 'KP', 'K', 'KE', 'E', 'LE', 'L', 'LP']
  const idx = Math.round(((deg + 22.5) % 360) / 45) % 8
  return dirs[idx]
}

export function tempColor(t: number | null | undefined): string {
  if (t == null) return 'text-gray-400'
  if (t < -10) return 'text-blue-300'
  if (t < 0) return 'text-blue-400'
  if (t < 10) return 'text-cyan-300'
  if (t < 20) return 'text-green-400'
  if (t < 28) return 'text-yellow-400'
  return 'text-orange-400'
}

export function tempChartColor(t: number): string {
  if (t < 0) return '#60a5fa'
  if (t < 10) return '#67e8f9'
  if (t < 20) return '#4ade80'
  return '#fb923c'
}
