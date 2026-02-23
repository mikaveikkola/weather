from pydantic_settings import BaseSettings
from typing import List


class Settings(BaseSettings):
    database_url: str = "postgresql+asyncpg://weather:weather@localhost:5432/weather"
    fmi_bbox: str = "19,59,32,71"
    fetch_interval_minutes: int = 10
    forecast_interval_minutes: int = 60
    default_places: str = "Helsinki,Tampere,Turku,Oulu,Rovaniemi"

    @property
    def places_list(self) -> List[str]:
        return [p.strip() for p in self.default_places.split(",")]

    model_config = {"env_file": ".env"}


settings = Settings()
