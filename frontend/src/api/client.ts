import axios, { type AxiosInstance } from 'axios'

export function createClient(baseURL: string): AxiosInstance {
  return axios.create({ baseURL, timeout: 15000 })
}

// Default client (Python backend via nginx) kept for any direct imports
export default createClient('/api/v1')
