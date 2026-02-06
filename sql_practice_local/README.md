# SQL Practice Platform (Local-Only)

A local-only SQL interview practice platform with **two alternative architectures**:

1. **[`kubernetes-with-python/`](./kubernetes-with-python/)** - Original Python microservices with K8s
2. **[`cplusplus/`](./cplusplus/)** - High-performance C++ monolithic server

Similar to DataLemur but designed for self-hosted, local development and practice.

## 🏗️ Choose Your Architecture

### Option 1: Python + Kubernetes (Original)
**Location:** [`kubernetes-with-python/`](./kubernetes-with-python/README.md)

- FastAPI backend with PostgreSQL
- Docker containers per query
- Horizontal scaling with K8s
- ~100 concurrent users per node
- 17-73s container startup

### Option 2: C++ Monolithic (High Performance)
**Location:** [`cplusplus/`](./cplusplus/README.md)

- Single C++ executable
- Embedded DuckDB database
- 10,000+ concurrent users per node
- <0.1s startup time
- 2,500-10,000x less memory

**Performance Comparison:**
| Metric | Python/K8s | C++ | Improvement |
|--------|------------|-----|-------------|
| Startup | 17-73s | <0.1s | **170-730x** |
| Memory/User | 512MB-2GB | 200KB | **2,500-10,000x** |
| 10K Users | ~5-20TB | ~2GB | **2,500-10,000x** |
| Max Concurrent | ~100/node | ~10,000+/node | **100x** |

---

## 🏗️ Architecture Overview (Python/Kubernetes)

```
┌─────────────────────────────────────────────────────────────────┐
│                       Local Browser                             │
│                   http://localhost:3000                         │
└─────────────────────────────────────────────────────────────────┘
                                  │
                                  │ HTTP/WebSocket
                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                    KUBERNETES CLUSTER                           │
│                      (kind / minikube)                          │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                  Frontend Pod (Next.js)                 │   │
│  │              Port: 3000 → NodePort: 30000              │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                  │                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │                Backend API Pod (FastAPI)                │   │
│  │              Port: 8000 → ClusterIP                    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                  │                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              SQL Engine Pod (FastAPI)                   │   │
│  │     Isolated PostgreSQL containers per query            │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                  │                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              PostgreSQL Database                        │   │
│  │              Port: 5432 → ClusterIP                    │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Redis Cache                                │   │
│  │              Port: 6379 → ClusterIP                    │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## 📦 Services

| Service | Technology | Port | Purpose |
|---------|-----------|------|---------|
| **Frontend** | Next.js 14 | 3000 | React UI with Monaco SQL Editor |
| **Backend API** | FastAPI (Python) | 8000 | REST API for auth, questions, progress |
| **SQL Engine** | FastAPI + Docker | 8001 | Secure SQL execution in isolated containers |
| **Database** | PostgreSQL | 5432 | Main data store |
| **Cache** | Redis | 6379 | Session & query caching |

## 🚀 Quick Start

### Prerequisites

- Docker Desktop (with Kubernetes enabled)
  OR
- [kind](https://kind.sigs.k8s.io/) (Kubernetes in Docker)
  OR
- [minikube](https://minikube.sigs.k8s.io/)

- kubectl CLI
- Node.js 20+
- Python 3.11+

### Option 1: Using Docker Compose (Simplest)

```bash
# Build and start all services
docker-compose up -d

# Access the application
open http://localhost:3000
```

### Option 2: Using Kubernetes (Recommended for Production-Like)

```bash
# Create kind cluster (if using kind)
kind create cluster --name sql-practice

# Or start minikube
minikube start

# Deploy to Kubernetes
kubectl apply -f k8s/base/

# Wait for pods to be ready
kubectl wait --for=condition=ready pod -l app=sql-practice -n sql-practice --timeout=300s

# Port forward to access services
kubectl port-forward -n sql-practice svc/frontend 3000:3000
```

### Development

```bash
# Frontend (in terminal 1)
cd frontend
npm install
npm run dev

# Backend (in terminal 2)
cd backend
pip install -r requirements.txt
uvicorn src.main:app --reload --port 8000

# SQL Engine (in terminal 3)
cd sql-engine
pip install -r requirements.txt
uvicorn src.main:app --reload --port 8001
```

## 📁 Project Structure

```
sql_practice_local/
├── frontend/              # Next.js frontend
│   ├── src/
│   │   ├── app/          # App router pages
│   │   ├── components/   # React components
│   │   ├── lib/          # Utilities
│   │   └── styles/       # Global styles
│   ├── package.json
│   └── Dockerfile
│
├── backend/              # FastAPI backend
│   ├── src/
│   │   ├── routes/       # API endpoints
│   │   ├── services/     # Business logic
│   │   ├── models/       # Database models
│   │   └── config/       # Configuration
│   ├── requirements.txt
│   └── Dockerfile
│
├── sql-engine/           # SQL execution engine
│   ├── src/
│   │   ├── core/         # Execution logic
│   │   └── utils/        # Docker management
│   ├── requirements.txt
│   └── Dockerfile
│
├── k8s/                  # Kubernetes manifests
│   ├── base/             # Base resources
│   │   ├── namespace.yaml
│   │   ├── deployment.yaml
│   │   ├── service.yaml
│   │   └── configmap.yaml
│   └── overlays/         # Environment-specific
│
├── db/                   # Database schemas
│   ├── init/             # Initialization scripts
│   ├── migrations/       # Migration files
│   └── schemas/          # SQL question schemas
│
├── docker/               # Docker configs
│   └── docker-compose.yml
│
├── scripts/              # Utility scripts
│   ├── setup-kind.sh
│   ├── deploy-k8s.sh
│   └── seed-data.py
│
└── docs/                 # Documentation
```

## 🔐 Security Features

- **SQL Sandboxing**: Each query executes in isolated Docker container
- **Query Timeouts**: 30-second execution limit
- **Resource Limits**: CPU (0.5 cores), Memory (512MB) per container
- **Read-Only Queries**: DROP, DELETE, UPDATE, INSERT blocked
- **Rate Limiting**: 10 queries per minute per user
- **Input Sanitization**: SQL injection prevention

## 📊 Features

- [x] Interactive SQL editor with syntax highlighting
- [x] Real-time query execution and results
- [x] Question catalog with difficulty levels
- [x] Progress tracking
- [x] Expected output comparison
- [x] Execution time measurement
- [ ] User authentication
- [ ] Premium hints system
- [ ] Submission history
- [ ] Performance analytics

## 🛠️ Tech Stack

### Frontend
- Next.js 14 (App Router)
- TypeScript
- Tailwind CSS
- Monaco Editor
- TanStack Query

### Backend
- FastAPI
- SQLAlchemy
- Pydantic
- PostgreSQL
- Redis
- Docker SDK

### Infrastructure
- Kubernetes (kind/minikube)
- Docker Compose
- PostgreSQL StatefulSet
- Redis Deployment

## 📝 License

MIT

## 🤝 Contributing

This is a local learning project. Feel free to fork and modify!
