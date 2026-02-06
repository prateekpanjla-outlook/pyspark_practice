"""
Backend API for SQL Practice Platform
FastAPI application serving REST API for authentication, questions, and user progress
"""

from fastapi import FastAPI, Depends, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.ext.asyncio import AsyncSession
import os

from src.config.database import get_db, engine
from src.config.settings import settings
from src.routes import questions, users, submissions

# Create FastAPI app
app = FastAPI(
    title="SQL Practice Platform API",
    description="Backend API for SQL interview practice platform",
    version="1.0.0"
)

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000", "http://frontend:3000"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Include routers
app.include_router(questions.router, prefix="/api/questions", tags=["questions"])
app.include_router(users.router, prefix="/api/users", tags=["users"])
app.include_router(submissions.router, prefix="/api/submissions", tags=["submissions"])


@app.on_event("startup")
async def startup_event():
    """Initialize database connection and create tables"""
    print("🚀 Starting SQL Practice Platform Backend")
    print(f"📦 Database URL: {settings.database_url}")


@app.on_event("shutdown")
async def shutdown_event():
    """Close database connections"""
    print("👋 Shutting down SQL Practice Platform Backend")


@app.get("/")
async def root():
    """Root endpoint"""
    return {
        "message": "SQL Practice Platform API",
        "version": "1.0.0",
        "docs": "/docs"
    }


@app.get("/health")
async def health_check(db: AsyncSession = Depends(get_db)):
    """Health check endpoint for Kubernetes probes"""
    try:
        # Simple database query to check connection
        await db.execute("SELECT 1")
        return {"status": "healthy", "database": "connected"}
    except Exception as e:
        raise HTTPException(status_code=503, detail=f"Unhealthy: {str(e)}")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app",
        host="0.0.0.0",
        port=8000,
        reload=True if os.getenv("ENVIRONMENT") == "development" else False
    )
