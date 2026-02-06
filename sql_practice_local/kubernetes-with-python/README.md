# SQL Practice Platform - Kubernetes with Python

## Architecture Overview

This is the **original Python-based architecture** using Kubernetes and Docker containers.

```
┌─────────────────────────────────────────────────────────────┐
│                   Kubernetes Cluster                        │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Backend API (FastAPI)                                │   │
│  │  - Serves frontend                                    │   │
│  │  - Manages user sessions                              │   │
│  │  - Fetches questions from PostgreSQL                  │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  SQL Execution Engine (FastAPI)                       │   │
│  │  - Spins up Docker container per query               │   │
│  │  - Executes user SQL in isolated PostgreSQL          │   │
│  │  - Compares result with expected output              │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  PostgreSQL Database                                  │   │
│  │  - Stores questions, users, submissions              │   │
│  └──────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Frontend (Next.js)                                   │   │
│  │  - SQL editor interface                               │   │
│  │  - Question browser                                   │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Directory Structure

```
kubernetes-with-python/
├── backend/                   # FastAPI backend service
│   ├── src/
│   │   ├── routes/           # API endpoints
│   │   ├── models/           # SQLAlchemy models
│   │   ├── config/           # Configuration
│   │   └── main.py           # Application entry
│   ├── Dockerfile
│   └── requirements.txt
│
├── sql-engine/               # SQL execution service
│   ├── src/
│   │   ├── core/
│   │   │   └── executor.py   # Docker container management
│   │   ├── config/
│   │   └── main.py           # Execution API
│   ├── Dockerfile
│   └── requirements.txt
│
├── frontend/                 # Next.js frontend
│   ├── src/
│   │   ├── components/
│   │   ├── pages/
│   │   └── styles/
│   ├── Dockerfile
│   └── package.json
│
├── k8s/                      # Kubernetes manifests
│   ├── base/
│   │   ├── deployment.yaml
│   │   ├── service.yaml
│   │   └── configmap.yaml
│   └── overlays/
│       ├── dev/
│       └── production/
│
├── docker/                   # Local Docker Compose
│   └── docker-compose.yml
│
├── db/                       # Database initialization
│   ├── init/
│   │   └── 01-init-database.sql
│   └── schemas/
│       └── sample_questions.sql
│
└── scripts/                  # Utility scripts
    ├── init-db.sh
    └── deploy.sh
```

## How It Works

### 1. User Requests SQL Question
```
Frontend → Backend API → PostgreSQL → Return Question + Schema
```

### 2. User Executes SQL
```
Frontend → Backend API → SQL Engine API
  ↓
SQL Engine spawns Docker container (postgres:16-alpine)
  ↓
Initialize schema + sample data
  ↓
Execute user SQL
  ↓
Compare with expected output
  ↓
Return result + is_correct flag
  ↓
Cleanup container
```

## Deployment

### Local (Docker Compose)
```bash
cd kubernetes-with-python/docker
docker-compose up -d
```

### Kubernetes
```bash
cd kubernetes-with-python/k8s
kubectl apply -k base/                    # Development
kubectl apply -k overlays/production/     # Production
```

## Configuration

### Backend (`backend/src/config/settings.py`)
```python
class Settings(BaseSettings):
    database_url: str = "postgresql://..."
    debug: bool = False
```

### SQL Engine (`sql-engine/src/config/settings.py`)
```python
class Settings(BaseSettings):
    max_containers: int = 10
    query_timeout: int = 30  # seconds
    memory_limit: str = "512m"
    cpu_limit: float = 0.5
    docker_image: str = "postgres:16-alpine"
```

## Performance Characteristics

| Metric | Value | Notes |
|--------|-------|-------|
| **Container startup** | 17-73s | PostgreSQL initialization |
| **Memory per query** | 512MB-2GB | Docker container overhead |
| **Max concurrent queries** | ~100 | Per node (limited by RAM) |
| **Query execution** | 50-500ms | Depends on complexity |
| **Scalability** | Horizontal | Add more K8s nodes |

## Pros & Cons

### ✅ Advantages
- **Strong isolation**: Full PostgreSQL in Docker per query
- **Horizontal scaling**: Kubernetes auto-scaling
- **Full PostgreSQL features**: Window functions, CTEs, etc.
- **Separation of concerns**: Microservices architecture

### ❌ Disadvantages
- **High startup overhead**: 17-73s per container
- **Resource intensive**: 512MB-2GB per query
- **Complex deployment**: Requires Kubernetes + Docker
- **Cost**: More infrastructure = higher cloud costs

## When to Use This Architecture

- ✅ Need production-grade microservices
- ✅ Existing Kubernetes infrastructure
- ✅ Need horizontal auto-scaling
- ✅ Want maximum query isolation
- ✅ Have DevOps team to manage K8s

## Migration to C++ Architecture

Consider migrating to `cplusplus/` if:
- Need to support 10,000+ concurrent users
- Want to reduce infrastructure costs
- Need faster query execution
- Don't require microservices separation
- Want simpler deployment (single binary)

## API Endpoints

### Backend API (Port 8000)
- `GET /api/questions` - List all questions
- `GET /api/questions/:slug` - Get question details
- `POST /api/submissions` - Submit solution
- `GET /api/leaderboard` - Get leaderboard

### SQL Engine API (Port 8001)
- `POST /api/execute` - Execute SQL query
- `POST /api/validate` - Validate SQL syntax
- `GET /health` - Health check

## Development

### Prerequisites
- Python 3.10+
- Docker
- Kubernetes (minikube/kind for local)
- Node.js 18+ (for frontend)

### Backend Development
```bash
cd backend
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
uvicorn src.main:app --reload
```

### SQL Engine Development
```bash
cd sql-engine
python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
uvicorn src.main:app --reload --port 8001
```

### Frontend Development
```bash
cd frontend
npm install
npm run dev
```

## Database Setup

```bash
# Start PostgreSQL
docker run -d --name sql-practice-db \
  -e POSTGRES_DB=sql_practice \
  -e POSTGRES_USER=sqlpractice \
  -e POSTGRES_PASSWORD=sqlpractice123 \
  -p 5432:5432 postgres:16-alpine

# Initialize schema
psql -h localhost -U sqlpractice -d sql_practice -f db/init/01-init-database.sql

# Load sample questions
psql -h localhost -U sqlpractice -d sql_practice -f db/schemas/sample_questions.sql
```

## Monitoring

### Kubernetes Metrics
```bash
kubectl top pods
kubectl logs -f deployment/backend
kubectl logs -f deployment/sql-engine
```

### Health Checks
```bash
curl http://localhost:8000/health   # Backend
curl http://localhost:8001/health   # SQL Engine
```

## Troubleshooting

### Container startup issues
```bash
# Check Docker logs
docker logs sql-sandbox-<id>

# Common issues:
# - Out of memory: Increase memory_limit
# - Timeout: Increase query_timeout
# - Port conflict: Check available ports
```

### Database connection issues
```bash
# Test PostgreSQL connection
psql -h localhost -U sqlpractice -d sql_practice

# Check backend database URL
echo $DATABASE_URL
```

## References

- [FastAPI Documentation](https://fastapi.tiangolo.com/)
- [Kubernetes Documentation](https://kubernetes.io/docs/)
- [Docker SDK for Python](https://docker-py.readthedocs.io/)
