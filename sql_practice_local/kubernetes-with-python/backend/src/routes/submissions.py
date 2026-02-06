"""
Submissions API routes
"""

from fastapi import APIRouter, Depends

router = APIRouter()


@router.post("/")
async def create_submission():
    """Submit a solution to a question"""
    return {"message": "Submissions not yet implemented"}


@router.get("/")
async def list_submissions():
    """List user's submissions"""
    return {"message": "Submissions not yet implemented"}
