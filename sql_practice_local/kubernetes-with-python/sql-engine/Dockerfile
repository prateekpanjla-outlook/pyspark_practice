# SQL Engine Dockerfile
FROM python:3.11-slim

WORKDIR /app

# Install system dependencies including Docker
RUN apt-get update && apt-get install -y \
    gcc \
    docker.io \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

# Copy requirements first for caching
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy application code
COPY . .

# Create non-root user with docker group access
RUN useradd -m -u 1000 sqlengine && \
    usermod -aG docker sqlengine && \
    chown -R sqlengine:sqlengine /app

# Note: In Kubernetes, we'll use securityContext to run with proper privileges
USER sqlengine

EXPOSE 8001

CMD ["uvicorn", "src.main:app", "--host", "0.0.0.0", "--port", "8001"]
