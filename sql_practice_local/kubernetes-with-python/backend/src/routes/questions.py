"""
Questions API routes
"""

from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from typing import List, Optional
from pydantic import BaseModel

from src.config.database import get_db

router = APIRouter()


# Pydantic models for request/response
class QuestionResponse(BaseModel):
    id: str
    title: str
    slug: str
    description: str
    difficulty: str
    category: str
    company: Optional[str] = None
    starter_code: Optional[str] = None
    tags: List[str] = []

    class Config:
        from_attributes = True


class QuestionDetailResponse(QuestionResponse):
    schema_data: dict
    hints: Optional[List[str]] = None
    solution: Optional[str] = None


@router.get("/", response_model=List[QuestionResponse])
async def list_questions(
    skip: int = Query(0, ge=0),
    limit: int = Query(20, ge=1, le=100),
    difficulty: Optional[str] = None,
    category: Optional[str] = None,
    tag: Optional[str] = None,
    db: AsyncSession = Depends(get_db)
):
    """
    List all questions with optional filtering

    - **skip**: Number of questions to skip (pagination)
    - **limit**: Maximum number of questions to return
    - **difficulty**: Filter by difficulty (easy, medium, hard)
    - **category**: Filter by category (sql, python, statistics, ml)
    - **tag**: Filter by tag (e.g., window-functions, joins)
    """
    from src.models.question import Question, QuestionTag

    query = select(Question).where(Question.is_active == True)

    # Apply filters
    if difficulty:
        query = query.where(Question.difficulty == difficulty)
    if category:
        query = query.where(Question.category == category)
    if tag:
        query = query.join(QuestionTag).where(QuestionTag.tag == tag)

    query = query.offset(skip).limit(limit).order_by(Question.difficulty, Question.title)

    result = await db.execute(query)
    questions = result.scalars().all()

    return questions


@router.get("/{slug}", response_model=QuestionDetailResponse)
async def get_question(
    slug: str,
    db: AsyncSession = Depends(get_db)
):
    """
    Get a specific question by slug

    - **slug**: Question slug (unique identifier)
    """
    from src.models.question import Question, QuestionTag

    # Get question
    result = await db.execute(
        select(Question).where(Question.slug == slug, Question.is_active == True)
    )
    question = result.scalar_one_or_none()

    if not question:
        raise HTTPException(status_code=404, detail="Question not found")

    # Get tags
    tags_result = await db.execute(
        select(QuestionTag.tag).where(QuestionTag.question_id == question.id)
    )
    tags = tags_result.scalars().all()

    response = QuestionDetailResponse(
        id=str(question.id),
        title=question.title,
        slug=question.slug,
        description=question.description,
        difficulty=question.difficulty,
        category=question.category,
        company=question.company,
        starter_code=question.starter_code,
        schema_data=question.schema_data,
        hints=question.hints if isinstance(question.hints, list) else None,
        solution=question.solution,
        tags=list(tags)
    )

    return response


@router.get("/tags/list")
async def list_tags(
    db: AsyncSession = Depends(get_db)
):
    """Get all available tags"""
    from src.models.question import QuestionTag

    result = await db.execute(
        select(QuestionTag.tag).distinct().order_by(QuestionTag.tag)
    )
    tags = result.scalars().all()

    return {"tags": list(tags)}
