# Docker Build Process - Detailed Trace

## Stage 1: Builder

### Step 1: Install Build Tools
```dockerfile
RUN apt-get update && apt-get install -y \
    build-essential \    # g++, gcc, make, etc.
    cmake \              # Build system
    git \                # For cloning Oat++
    wget \               # For downloading DuckDB
    libssl-dev \         # OpenSSL headers
    pkg-config
```
**Result:** ~500MB of tools and libraries

### Step 2: Download DuckDB
```dockerfile
RUN wget -q https://github.com/duckdb/duckdb/releases/download/v1.0.0/libduckdb-linux-amd64.tar.gz
RUN tar -xzf libduckdb-linux-amd64.tar.gz
RUN cp libduckdb-linux-amd64/*.so* /usr/local/lib/
RUN cp libduckdb-linux-amd64/*.h /usr/local/include/
RUN ldconfig
```
**Result:** DuckDB library available system-wide

### Step 3: Build Oat++
```dockerfile
RUN git clone --depth 1 --branch 1.3.0 https://github.com/oatpp/oatpp.git
RUN mkdir oatpp/build && cd oatpp/build
RUN cmake ..
RUN make -j$(nproc)
RUN make install
```
**Result:** Oat++ HTTP framework installed

### Step 4: Copy Your Code
```dockerfile
WORKDIR /app
COPY CMakeLists.txt ./
COPY src/ ./src/
```
**Result:** Your source code is now in the container

### Step 5: Build Your Application
```dockerfile
RUN mkdir build && cd build
RUN cmake ..
RUN make -j$(nproc)
```
**What happens:**
```
/app/build/
├── sql-practice-server    ← The compiled binary
└── object files (*.o)
```

---

## Stage 2: Runtime

### Copy ONLY What's Needed
```dockerfile
FROM ubuntu:22.04

# Minimal runtime deps
RUN apt-get install -y libssl3 ca-certificates

# Copy binary from builder
COPY --from=builder /app/build/sql-practice-server /usr/local/bin/

# Copy DuckDB library
COPY --from=builder /usr/local/lib/libduckdb.so* /usr/local/lib/
```

**Result:** ~100MB image (vs ~2GB if we kept build tools)

---

## Complete Build Timeline

```
Time  Action
─────────────────────────────────────────────────
0:00  Start ubuntu:22.04 base image
0:15  Install build tools (build-essential, cmake)
1:00  Download DuckDB library
1:20  Clone Oat++ repository
2:00  Build Oat++ (make -j$(nproc))
2:30  Copy your source code
3:00  CMake configure
4:00  Compile your code (all 7 .cpp files)
4:05  Create runtime image
4:06  Complete!
```

**Total time:** ~4-5 minutes on first build (cached afterward)

---

## Docker Layer Caching

```
Layer 1: Base image (ubuntu:22.04)           ← Cached
Layer 2: Install build tools                ← Cached
Layer 3: Download DuckDB                    ← Cached
Layer 4: Build Oat++                        ← Cached
Layer 5: COPY source code                   ← Invalidated on code change!
Layer 6: Build application                  ← Rebuilt
Layer 7: Runtime image copy                 ← Rebuilt
```

**Key insight:** Only steps 5-7 repeat when you change code. Steps 1-4 are cached.

---

## Troubleshooting

### "CMake not found"
```dockerfile
# Already installed in stage 1
RUN apt-get install -y cmake
```

### "DuckDB headers not found"
```dockerfile
# Make sure this copied correctly
COPY --from=builder /usr/local/lib/libduckdb.so* /usr/local/lib/
```

### Build fails with compilation error
```bash
# Check build logs
docker build -t sql-practice-cpp -f docker/Dockerfile . 2>&1 | tee build.log

# Look for errors in build.log
grep -i error build.log
```

### Container exits immediately
```bash
# Check logs
docker logs sql-practice-cpp

# Run interactively to debug
docker run -it sql-practice-cpp /bin/bash
./usr/local/bin/sql-practice-server --help
```

---

## Production Optimization

### 1. Use Specific Version Tags
```dockerfile
FROM ubuntu:22.04 AS builder
# Instead of FROM ubuntu:latest
```

### 2. Multi-arch Builds
```bash
# Build for AMD64 and ARM64
docker buildx build --platform linux/amd64,linux/arm64 -t sql-practice-cpp .
```

### 3. Use BuildKit for Faster Builds
```bash
# Enable BuildKit
export DOCKER_BUILDKIT=1

# Build with caching
docker build --cache-from sql-practice-cpp:latest -t sql-practice-cpp .
```

### 4. Minimal Base Image
```dockerfile
# Use Alpine instead of Ubuntu (smaller but requires changes)
FROM alpine:3.19 AS builder
# Note: Would need to adjust many commands for Alpine
```

---

## Quick Reference

```bash
# Build
docker build -t sql-practice-cpp -f docker/Dockerfile .

# Run
docker run -d -p 8080:8080 sql-practice-cpp

# Logs
docker logs -f sql-practice-cpp

# Test
curl http://localhost:8080/health

# Shell access
docker run -it sql-practice-cpp /bin/bash

# Cleanup
docker stop sql-practice-cpp
docker rm sql-practice-cpp
docker rmi sql-practice-cpp
```
