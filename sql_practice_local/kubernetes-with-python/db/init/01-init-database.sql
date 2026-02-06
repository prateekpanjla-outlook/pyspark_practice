-- Initialize database schema for SQL Practice Platform

-- Enable required extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";

-- Create custom types
CREATE TYPE user_role AS ENUM ('free', 'premium', 'admin');
CREATE TYPE difficulty_level AS ENUM ('easy', 'medium', 'hard');
CREATE TYPE question_category AS ENUM ('sql', 'python', 'statistics', 'ml');

-- Users table
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    email VARCHAR(255) UNIQUE NOT NULL,
    username VARCHAR(50) UNIQUE,
    password_hash VARCHAR(255),
    role user_role DEFAULT 'free',
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Questions table
CREATE TABLE questions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    title VARCHAR(255) NOT NULL,
    slug VARCHAR(255) UNIQUE NOT NULL,
    description TEXT NOT NULL,
    difficulty difficulty_level NOT NULL,
    category question_category NOT NULL DEFAULT 'sql',
    company VARCHAR(100),
    schema_data JSONB NOT NULL,
    expected_output JSONB NOT NULL,
    starter_code TEXT,
    hints JSONB,
    solution TEXT,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Question tags (many-to-many)
CREATE TABLE question_tags (
    question_id UUID REFERENCES questions(id) ON DELETE CASCADE,
    tag VARCHAR(50) NOT NULL,
    PRIMARY KEY (question_id, tag)
);

-- User submissions
CREATE TABLE submissions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    question_id UUID REFERENCES questions(id) ON DELETE CASCADE,
    user_sql TEXT NOT NULL,
    is_correct BOOLEAN NOT NULL,
    execution_time_ms INTEGER,
    error_message TEXT,
    submitted_at TIMESTAMP DEFAULT NOW()
);

-- User progress (aggregated)
CREATE TABLE user_progress (
    user_id UUID PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    easy_completed INTEGER DEFAULT 0,
    medium_completed INTEGER DEFAULT 0,
    hard_completed INTEGER DEFAULT 0,
    total_attempts INTEGER DEFAULT 0,
    last_practiced_at TIMESTAMP,
    streak_days INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Create indexes for performance
CREATE INDEX idx_submissions_user_question ON submissions(user_id, question_id);
CREATE INDEX idx_submissions_user_date ON submissions(user_id, submitted_at DESC);
CREATE INDEX idx_questions_difficulty ON questions(difficulty);
CREATE INDEX idx_questions_category ON questions(category);
CREATE INDEX idx_questions_company ON questions(company);
CREATE INDEX idx_questions_active ON questions(is_active);
CREATE INDEX idx_question_tags_tag ON question_tags(tag);
CREATE INDEX idx_users_email ON users(email);

-- Insert sample data (seed user for testing)
INSERT INTO users (email, username, role) VALUES
('demo@example.com', 'demo_user', 'free');

-- Function to update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Triggers for updated_at
CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_questions_updated_at BEFORE UPDATE ON questions
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_user_progress_updated_at BEFORE UPDATE ON user_progress
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE questions IS 'Catalog of SQL and coding practice questions';
COMMENT ON TABLE submissions IS 'User code submissions for practice questions';
COMMENT ON TABLE user_progress IS 'Aggregated user progress and statistics';
