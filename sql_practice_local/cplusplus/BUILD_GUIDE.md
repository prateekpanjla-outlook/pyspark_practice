# C++ Build Guide

## Prerequisites

The C++ version requires several dependencies to be installed. Choose your platform:

---

## Windows (Visual Studio)

### 1. Install Visual Studio 2022
- Download: https://visualstudio.microsoft.com/downloads/
- Select "Desktop development with C++" workload
- Includes: MSVC compiler, CMake (optional, VS has built-in CMake)

### 2. Install vcpkg (Package Manager)
```powershell
# In PowerShell
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

### 3. Install Dependencies via vcpkg
```powershell
# DuckDB
.\vcpkg\vcpkg install duckdb:x64-windows

# Oat++ (HTTP framework)
.\vcpkg\vcpkg install oatpp:x64-windows

# nlohmann/json
.\vcpkg\vcpkg install nlohmann-json:x64-windows
```

### 4. Build
```powershell
cd cplusplus
mkdir build
cd build

# Configure with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release
```

### 5. Run
```powershell
.\Release\sql-practice-server.exe
```

---

## Windows (MinGW-w64)

### 1. Install MSYS2
- Download: https://www.msys2.org/
- Run MSYS2 and update:
```bash
pacman -Syu
```

### 2. Install Build Tools
```bash
# In MSYS2 MinGW 64-bit shell
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
```

### 3. Install DuckDB
```bash
# Download prebuilt library
wget https://github.com/duckdb/duckdb/releases/download/v1.0.0/libduckdb-windows-amd64.zip
unzip libduckdb-windows-amd64.zip
cp libduckdb.dll /c/Windows/System32/
cp duckdb.h /mingw64/include/
cp libduckdb.dll.a /mingw64/lib/
```

### 4. Build
```bash
cd /c/Users/prateek/Desktop/spark/pyspark_practice/sql_practice_local/cplusplus
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

---

## Linux (Ubuntu/Debian)

### 1. Install Build Tools
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git pkg-config
```

### 2. Install DuckDB
```bash
wget https://github.com/duckdb/duckdb/releases/download/v1.0.0/libduckdb-linux-amd64.zip
unzip libduckdb-linux-amd64.zip
sudo cp libduckdb-linux-amd64/*.so* /usr/local/lib/
sudo cp libduckdb-linux-amd64/*.h /usr/local/include/
sudo ldconfig
```

### 3. Build from Source (Simpler)
```bash
cd cplusplus
mkdir build && cd build
cmake ..
make -j8
```

### 4. Run
```bash
./sql-practice-server
```

---

## macOS

### 1. Install Xcode Command Line Tools
```bash
xcode-select --install
```

### 2. Install Homebrew
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Install Dependencies
```bash
brew install cmake duckdb
```

### 4. Build
```bash
cd cplusplus
mkdir build && cd build
cmake ..
make -j8
```

---

## Docker Build (Universal)

### Build Docker Image
```bash
cd cplusplus/docker
docker build -t sql-practice-cpp ..
```

### Run Container
```bash
docker run -p 8080:8080 sql-practice-cpp
```

---

## Troubleshooting

### "duckdb.h not found"
**Solution:** Install DuckDB library (see platform-specific instructions above)

### "oatpp not found"
**Solution:** Either:
1. Install Oat++ via package manager
2. Or use `-DUSE_SYSTEM_OATPP=OFF` to let CMake download it

### "CMake not found"
**Solution:** Install CMake:
- Windows: Install Visual Studio or standalone CMake
- Linux: `sudo apt-get install cmake`
- macOS: `brew install cmake`

### Build Errors
**Common fixes:**
```bash
# Clean build
cd build
rm -rf *
cmake ..
make clean
make -j8

# Increase CMake verbosity
cmake .. --trace-expand

# Check C++ version
g++ --version  # Should be 7.0+ for C++17
```

---

## Quick Start (Docker - Recommended for Testing)

If you just want to test without installing dependencies:

```bash
cd cplusplus/docker
docker-compose up
```

Access at: http://localhost:8080

---

## Project Structure Reference

```
cplusplus/
├── src/
│   ├── main.cpp                 # Entry point
│   ├── include/                 # Headers
│   │   ├── session_manager.hpp
│   │   ├── sql_executor.hpp
│   │   ├── question_loader.hpp
│   │   └── http_server.hpp
│   ├── core/                    # Core logic
│   ├── db/                      # Database layer
│   │   └── embedded_questions.cpp  # ← Add questions here
│   └── http/                    # HTTP handlers
├── CMakeLists.txt               # Build configuration
└── docker/                      # Docker deployment
```

---

## Adding New Questions

Edit `src/db/embedded_questions.cpp`:

```cpp
static const QuestionDef my_new_question = {
    "q4",
    "Question Title",
    "question-slug",
    "Description",
    "medium",
    "sql",
    "Company",
    "-- Starter code",
    "-- Solution",
    {"tag1", "tag2"},
    {"Hint 1", "Hint 2"},
    { /* schema */ },
    { /* sample data */ },
    { /* expected output */ }
};

// Add to ALL_QUESTIONS array
static const std::vector<QuestionDef> ALL_QUESTIONS = {
    question_1,
    question_2,
    question_3,
    my_new_question  // ← Add here
};
```

Then rebuild:
```bash
cd build
make -j8
```
