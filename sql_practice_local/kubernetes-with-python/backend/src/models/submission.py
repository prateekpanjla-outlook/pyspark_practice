"""
Submission models
"""

from sqlalchemy import Column, String, Integer, Text, Boolean, DateTime, ForeignKey
from sqlalchemy.dialects.postgresql import UUID
from datetime import datetime
import uuid

from .user import Base


class Submission(Base):
    __tablename__ = "submissions"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    user_id = Column(UUID(as_uuid=True), ForeignKey("users.id", ondelete="CASCADE"), nullable=False)
    question_id = Column(UUID(as_uuid=True), ForeignKey("questions.id", ondelete="CASCADE"), nullable=False)
    user_sql = Column(Text, nullable=False)
    is_correct = Column(Boolean, nullable=False)
    execution_time_ms = Column(Integer)
    error_message = Column(Text)
    submitted_at = Column(DateTime, default=datetime.utcnow)
