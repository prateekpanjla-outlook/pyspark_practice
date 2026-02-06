# Building Linux Executable from Windows

## Option 1: Docker Build (Recommended)

The Dockerfile produces a **Linux binary** - this is already configured!

### Prerequisites
Install Docker Desktop for Windows: https://www.docker.com/products/docker-desktop/

### Build Linux Executable
```bash
cd cplusplus
docker build -t sql-practice-cpp -f docker/Dockerfile .

# Extract the binary from the image
docker create --name temp sql-practice-cpp
docker cp temp:/usr/local/bin/sql-practice-server ./sql-practice-server
docker rm temp
```

### Run Linux Binary
```bash
# In Docker
docker run -p 8080:8080 sql-practice-cpp

# Or copy binary to Linux server and run
./sql-practice-server --port 8080
```

---

## Option 2: WSL2 (Native Linux Build on Windows)

### Install WSL2
```powershell
# In PowerShell (Admin)
wsl --install
```
This installs Ubuntu on Windows.

### Build in WSL2
```bash
# In WSL2 Ubuntu terminal
cd /mnt/c/Users/prateek/Desktop/spark/pyspark_practice/sql_practice_local/cplusplus

# Install build tools
sudo apt-get update
sudo apt-get install -y build-essential cmake git wget

# Build
mkdir build && cd build
cmake ..
make -j8

# Run
./sql-practice-server
```

---

## Option 3: GitHub Actions (Cloud Build)

Push to GitHub and let Actions build for you:

```yaml
# .github/workflows/build.yml
name: Build Linux Binary

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake g++ wget git
      - name: Build
        run: |
          cd cplusplus
          mkdir build && cd build
          cmake ..
          make -j8
      - name: Upload binary
        uses: actions/upload-artifact@v3
        with:
          name: sql-practice-server-linux
          path: cplusplus/build/sql-practice-server
```

---

## Comparison

| Option | Produces | Speed | Setup |
|--------|----------|-------|-------|
| **Docker** | Linux binary | ~4 min | Install Docker |
| **WSL2** | Linux binary | ~2 min | Install WSL2 |
| **GitHub Actions** | Linux binary | ~5 min | Push to GitHub |

---

## Why Linux Binary?

| Aspect | Windows (.exe) | Linux |
|--------|----------------|-------|
| **Deployment** | Windows servers | Linux/Docker/cloud |
| **Production** | Rare | Industry standard |
| **Docker** | Windows containers | Linux containers (99%) |
| **Cloud** | AWS EC2 Windows | AWS/Azure/GCP Linux |
| **Size** | Larger | Smaller |

---

## Recommended Approach

**Use Docker build** - it's already configured and produces a production-ready Linux container!

```bash
cd cplusplus/docker
docker build -t sql-practice-cpp ..
docker save sql-practice-cpp > sql-practice-cpp.tar

# Deploy to any Linux server
docker load < sql-practice-cpp.tar
docker run -d -p 8080:8080 sql-practice-cpp
```
