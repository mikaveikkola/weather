import { BackendProvider } from './contexts/BackendContext'
import Dashboard from './pages/Dashboard'

export default function App() {
  return (
    <BackendProvider>
      <Dashboard />
    </BackendProvider>
  )
}
