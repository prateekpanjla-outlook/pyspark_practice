#!/bin/bash
# Deploy SQL Practice Platform to Kubernetes

set -e

echo "🚀 Deploying SQL Practice Platform to Kubernetes..."

# Create namespace
echo "📦 Creating namespace..."
kubectl apply -f k8s/base/namespace.yaml

# Deploy PostgreSQL
echo "🐘 Deploying PostgreSQL..."
kubectl apply -f k8s/base/postgres.yaml

# Deploy Redis
echo "🔴 Deploying Redis..."
kubectl apply -f k8s/base/redis.yaml

# Wait for database to be ready
echo "⏳ Waiting for database to be ready..."
kubectl wait --for=condition=ready pod -l app=postgres -n sql-practice --timeout=120s

# Deploy backend
echo "🔧 Deploying backend..."
kubectl apply -f k8s/base/backend.yaml

# Deploy SQL engine
echo "🔧 Deploying SQL engine..."
kubectl apply -f k8s/base/sql-engine.yaml

# Deploy frontend
echo "🎨 Deploying frontend..."
kubectl apply -f k8s/base/frontend.yaml

echo "✅ Deployment complete!"
echo ""
echo "📊 To check status:"
echo "  kubectl get pods -n sql-practice"
echo ""
echo "📡 To access the application:"
echo "  kubectl port-forward -n sql-practice svc/frontend 3000:3000"
echo "  Then open: http://localhost:3000"
