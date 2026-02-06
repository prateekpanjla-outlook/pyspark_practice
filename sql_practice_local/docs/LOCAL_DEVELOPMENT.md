# Local Development Guide

This guide covers setting up and running the SQL Practice Platform locally.

## Option 1: Docker Compose (Recommended for Development)

### Prerequisites
- Docker Desktop installed
- Docker Compose installed

### Steps

1. **Build and start all services:**
   ```bash
   cd docker
   docker-compose up -d
   ```

2. **View logs:**
   ```bash
   docker-compose logs -f
   ```

3. **Stop services:**
   ```bash
   docker-compose down
   ```

4. **Access the application:**
   - Frontend: http://localhost:3000
   - Backend API: http://localhost:8000
   - API Docs: http://localhost:8000/docs
   - SQL Engine: http://localhost:8001

### Development Mode

To run services in development mode with hot-reload:

1. **Start infrastructure only:**
   ```bash
   cd docker
   docker-compose up -d postgres redis
   ```

2. **Run backend locally:**
   ```bash
   cd backend
   pip install -r requirements.txt
   uvicorn src.main:app --reload --port 8000
   ```

3. **Run SQL engine locally:**
   ```bash
   cd sql-engine
   pip install -r requirements.txt
   uvicorn src.main:app --reload --port 8001
   ```

4. **Run frontend locally:**
   ```bash
   cd frontend
   npm install
   npm run dev
   ```

---

## Option 2: Kubernetes (Production-Like)

### Prerequisites

Choose one:
- [Docker Desktop with Kubernetes enabled](https://docs.docker.com/desktop/kubernetes/)
- [kind (Kubernetes in Docker)](https://kind.sigs.k8s.io/)
- [minikube](https://minikube.sigs.k8s.io/)

### Using kind

1. **Create cluster:**
   ```bash
   chmod +x scripts/setup-kind.sh
   ./scripts/setup-kind.sh
   ```

2. **Deploy application:**
   ```bash
   chmod +x scripts/deploy-k8s.sh
   ./scripts/deploy-k8s.sh
   ```

3. **Access via NodePort:**
   ```bash
   # Frontend is exposed on NodePort 30000
   open http://localhost:30000
   ```

4. **Access via port-forward:**
   ```bash
   kubectl port-forward -n sql-practice svc/frontend 3000:3000
   open http://localhost:3000
   ```

### Using minikube

1. **Start minikube:**
   ```bash
   minikube start
   ```

2. **Deploy application:**
   ```bash
   chmod +x scripts/deploy-k8s.sh
   ./scripts/deploy-k8s.sh
   ```

3. **Access via minikube tunnel:**
   ```bash
   minikube tunnel
   ```

### Common Kubernetes Commands

```bash
# View pods
kubectl get pods -n sql-practice

# View logs
kubectl logs -f deployment/backend -n sql-practice

# Get services
kubectl get svc -n sql-practice

# Port forward to local
kubectl port-forward -n sql-practice svc/frontend 3000:3000
kubectl port-forward -n sql-practice svc/backend 8000:8000

# Scale deployment
kubectl scale deployment/backend --replicas=3 -n sql-practice

# Restart deployment
kubectl rollout restart deployment/backend -n sql-practice
```

---

## Database Setup

### Initialize with sample data

```bash
# After PostgreSQL is running, execute initialization scripts
psql -h localhost -U sqlpractice -d sql_practice -f db/init/01-init-database.sql
psql -h localhost -U sqlpractice -d sql_practice -f db/schemas/sample_questions.sql
```

### Connect to database

```bash
# Using psql
psql -h localhost -U sqlpractice -d sql_practice

# Using Docker
docker exec -it sql-practice-db psql -U sqlpractice -d sql_practice
```

---

## Troubleshooting

### PostgreSQL connection issues

If backend can't connect to PostgreSQL:
- Check PostgreSQL is running: `docker ps | grep postgres`
- Check connection string in `.env`
- Verify network connectivity

### SQL Engine Docker issues

If SQL engine can't create containers:
- Ensure Docker socket is mounted: `-v /var/run/docker.sock:/var/run/docker.sock`
- Check user has Docker permissions
- Verify `docker ps` works from within container

### Port already in use

If ports are already in use:
```bash
# Check what's using the port
lsof -i :3000
lsof -i :8000

# Change ports in docker-compose.yml if needed
```

### Kubernetes pods not starting

```bash
# Describe pod for more details
kubectl describe pod <pod-name> -n sql-practice

# View logs
kubectl logs <pod-name> -n sql-practice

# Check events
kubectl get events -n sql-practice
```

---

## Testing

### Test Backend API

```bash
# Health check
curl http://localhost:8000/health

# List questions
curl http://localhost:8000/api/questions

# Get specific question
curl http://localhost:8000/api/questions/second-highest-salary
```

### Test SQL Engine

```bash
curl -X POST http://localhost:8001/api/execute \
  -H "Content-Type: application/json" \
  -d '{
    "question_id": "test",
    "user_sql": "SELECT 1",
    "schema_data": {"tables": [], "sample_data": {}}
  }'
```

---

## Next Steps

1. Add user authentication
2. Implement frontend SQL editor
3. Add more practice questions
4. Implement submission history
5. Add progress tracking dashboard
6. Deploy to production
