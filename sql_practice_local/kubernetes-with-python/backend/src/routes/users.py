"""
Users API routes
"""

from fastapi import APIRouter, Depends

router = APIRouter()


@router.get("/me")
async def get_current_user():
    """Get current user profile"""
    return {"message": "User authentication not yet implemented"}
