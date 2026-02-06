# Multi-stage build for SQL Practice C++ Server
# Results in minimal image with just the binary

# Stage 1: Build stage
FROM ubuntu:22.04 AS builder

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libssl-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Download and install DuckDB
WORKDIR /tmp
RUN wget -q https://github.com/duckdb/duckdb/releases/download/v1.0.0/libduckdb-linux-amd64.tar.gz \
    && tar -xzf libduckdb-linux-amd64.tar.gz \
    && cp libduckdb-linux-amd64/*.so* /usr/local/lib/ \
    && cp libduckdb-linux-amd64/*.h /usr/local/include/ \
    && ldconfig

# Clone and build Oat++
WORKDIR /tmp
RUN git clone --depth 1 --branch 1.3.0 https://github.com/oatpp/oatpp.git \
    && mkdir oatpp/build \
    && cd oatpp/build \
    && cmake .. \
    && make -j$(nproc) \
    && make install \
    && ldconfig

# Copy source code (Pure C++ - no questions folder needed)
WORKDIR /app
COPY CMakeLists.txt ./
COPY src/ ./src/

# Build the application
RUN mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && make install

# Stage 2: Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy DuckDB library
COPY --from=builder /usr/local/lib/libduckdb.so* /usr/local/lib/

# Copy the binary
COPY --from=builder /app/build/sql-practice-server /usr/local/bin/

# Set library path
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Run the server
ENTRYPOINT ["sql-practice-server"]
CMD ["--port", "8080"]
