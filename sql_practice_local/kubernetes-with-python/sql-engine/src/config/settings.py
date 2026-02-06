"""
SQL Engine configuration
"""

from pydantic_settings import BaseSettings
from functools import lru_cache


class Settings(BaseSettings):
    """SQL Engine settings"""

    # Database (for fetching questions)
    database_url: str = "postgresql://sqlpractice:sqlpractice123@localhost:5432/sql_practice"

    # Container limits
    max_containers: int = 10
    query_timeout: int = 30  # seconds
    memory_limit: str = "512m"
    cpu_limit: float = 0.5

    # Docker settings
    docker_image: str = "postgres:16-alpine"
    container_prefix: str = "sql-sandbox"

    class Config:
        env_file = ".env"
        case_sensitive = False


@lru_cache()
def get_settings() -> Settings:
    """Get cached settings instance"""
    return Settings()


settings = get_settings()
