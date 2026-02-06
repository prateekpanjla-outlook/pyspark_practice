"""
Question models
"""

from sqlalchemy import Column, String, Text, DateTime, Boolean, ForeignKey, Enum
from sqlalchemy.dialects.postgresql import UUID, JSONB
from sqlalchemy.orm import relationship
from datetime import datetime
import uuid

from .user import Base


class Question(Base):
    __tablename__ = "questions"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    title = Column(String(255), nullable=False)
    slug = Column(String(255), unique=True, nullable=False)
    description = Column(Text, nullable=False)
    difficulty = Column(Enum("easy", "medium", "hard", name="difficulty_level"), nullable=False)
    category = Column(Enum("sql", "python", "statistics", "ml", name="question_category"), default="sql")
    company = Column(String(100))
    schema_data = Column(JSONB, nullable=False)
    expected_output = Column(JSONB, nullable=False)
    starter_code = Column(Text)
    hints = Column(JSONB)
    solution = Column(Text)
    is_active = Column(Boolean, default=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)


class QuestionTag(Base):
    __tablename__ = "question_tags"

    question_id = Column(UUID(as_uuid=True), ForeignKey("questions.id", ondelete="CASCADE"), primary_key=True)
    tag = Column(String(50), primary_key=True)
