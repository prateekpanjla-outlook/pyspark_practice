"""
Application settings and configuration
"""

from pydantic_settings import BaseSettings
from functools import lru_cache


class Settings(BaseSettings):
    """Application settings"""

    # Database
    database_url: str = "postgresql://sqlpractice:sqlpractice123@localhost:5432/sql_practice"

    # Redis
    redis_url: str = "redis://localhost:6379"

    # SQL Engine
    sql_engine_url: str = "http://localhost:8001"

    # Security
    jwt_secret: str = "your-secret-key-change-in-production"
    jwt_algorithm: str = "HS256"
    access_token_expire_minutes: int = 60 * 24  # 24 hours

    # Application
    app_name: str = "SQL Practice Platform"
    environment: str = "development"
    debug: bool = True

    # Pagination
    default_page_size: int = 20
    max_page_size: int = 100

    class Config:
        env_file = ".env"
        case_sensitive = False


@lru_cache()
def get_settings() -> Settings:
    """Get cached settings instance"""
    return Settings()


# Export settings instance
settings = get_settings()
