#!/bin/bash
# Setup kind cluster for SQL Practice Platform

set -e

CLUSTER_NAME="sql-practice"

echo "🔧 Creating kind cluster: $CLUSTER_NAME"

# Create cluster with extra port mappings for NodePort access
kind create cluster \
  --name $CLUSTER_NAME \
  --config - <<EOF
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
nodes:
  - role: control-plane
    extraPortMappings:
      - containerPort: 30000
        hostPort: 30000
        protocol: TCP
EOF

echo "✅ Cluster created successfully!"
echo "📡 To access the frontend, use: http://localhost:30000"
