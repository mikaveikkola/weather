import { createContext, useContext, useState, type ReactNode } from 'react'

export type Backend = 'python' | 'cpp'

interface BackendContextType {
  backend: Backend
  baseUrl: string
  setBackend: (b: Backend) => void
}

const BACKEND_URLS: Record<Backend, string> = {
  python: '/api/v1',
  cpp: 'http://localhost:8001/api/v1',
}

const BackendContext = createContext<BackendContextType>({
  backend: 'python',
  baseUrl: BACKEND_URLS.python,
  setBackend: () => {},
})

export function BackendProvider({ children }: { children: ReactNode }) {
  const [backend, setBackendState] = useState<Backend>(
    () => (localStorage.getItem('backend') as Backend) || 'python',
  )

  const setBackend = (b: Backend) => {
    localStorage.setItem('backend', b)
    setBackendState(b)
  }

  return (
    <BackendContext.Provider value={{ backend, baseUrl: BACKEND_URLS[backend], setBackend }}>
      {children}
    </BackendContext.Provider>
  )
}

export const useBackend = () => useContext(BackendContext)
