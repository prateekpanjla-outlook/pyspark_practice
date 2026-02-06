# Installing Clang for Windows & Building C++ Project

## Step 1: Install LLVM/Clang

### Download LLVM
1. Go to: https://github.com/llvm/llvm-project/releases
2. Download the latest **LLVM-XX.X.X-win64.exe** installer
   - Example: `LLVM-18.1.8-win64.exe`
   - Size: ~200-300 MB

### Install
1. Run the installer as Administrator
2. Install to: `C:\Program Files\LLVM`
3. ✅ **Check "Add LLVM to the system PATH for current user"**
4. Click Install

### Verify Installation
Open **Command Prompt** (cmd.exe) or PowerShell:
```cmd
clang --version
clang++ --version
```

Expected output:
```
clang version 18.1.8
Target: x86_64-pc-windows-msvc
Thread model: posix
```

---

## Step 2: Install CMake

### Download CMake
1. Go to: https://cmake.org/download/
2. Download: `cmake-X.X.X-windows-x86_64.msi`
3. Run installer
4. ✅ **Check "Add CMake to the system PATH"**

### Verify
```cmd
cmake --version
```

---

## Step 3: Install Ninja (Optional but Recommended)

### Why Ninja?
- **Faster builds** than Visual Studio project files
- **Better integration** with Clang
- **Simpler** CMake configuration

### Install Ninja
**Option A: Using Chocolatey** (if you have it)
```powershell
choco install ninja
```

**Option B: Manual download**
1. Go to: https://github.com/ninja-build/ninja/releases
2. Download `ninja-win.zip`
3. Extract to: `C:\ninja`
4. **Add to PATH**: `C:\ninja`
   - Search for "Edit the system environment variables"
   - Click "Environment Variables"
   - Edit "Path" → Add `C:\ninja`

### Verify
```cmd
ninja --version
```

---

## Step 4: Install Build Dependencies

The project needs **DuckDB** and **Oat++**.

### Option A: Let CMake Download (Easiest)

The CMakeLists.txt is configured to automatically download Oat++ and nlohmann/json using FetchContent.

**Only DuckDB needs manual installation:**

1. Download DuckDB:
   - https://github.com/duckdb/duckdb/releases/download/v1.0.0/libduckdb-windows-amd64.zip

2. Extract and copy files:
   ```cmd
   mkdir C:\duckdb
   # Extract zip to C:\duckdb

   # Copy to LLVM directory
   copy C:\duckdb\duckdb.h "C:\Program Files\LLVM\include\"
   copy C:\duckdb\duckdb.dll "C:\Program Files\LLVM\bin\"
   copy C:\duckdb\libduckdb.dll.lib "C:\Program Files\LLVM\lib\"
   ```

### Option B: Manual Install (If FetchContent fails)

1. **Install Oat++**:
   ```cmd
   git clone https://github.com/oatpp/oatpp.git C:\oatpp
   cd C:\oatpp
   mkdir build && cd build
   cmake ..
   cmake --install . --prefix C:\oatpp-install
   ```

2. **Install nlohmann/json**:
   - Download: https://github.com/nlohmann/json/releases
   - Extract `single_include/nlohmann/json.hpp` to:
     ```
     C:\Program Files\LLVM\include\nlohmann\json.hpp
     ```

---

## Step 5: Build the Project

### Open Command Prompt
```cmd
# Navigate to project
cd C:\Users\prateek\Desktop\spark\pyspark_practice\sql_practice_local\cplusplus

# Create build directory
mkdir build
cd build
```

### Configure with CMake
```cmd
# Using Ninja (recommended)
cmake .. -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

# OR using Visual Studio generator (with ClangCL)
cmake .. -G "Visual Studio 17 2022" -A x64 -T ClangCL
```

### Build
```cmd
# If using Ninja
ninja

# If using Visual Studio generator
cmake --build . --config Release
```

### Run
```cmd
# From build directory
.\sql-practice-server.exe

# OR from Release folder
.\Release\sql-practice-server.exe
```

---

## Troubleshooting

### "clang not found"
**Solution:**
- Restart Command Prompt after installing LLVM
- Verify PATH includes: `C:\Program Files\LLVM\bin`

### "CMake cannot find DuckDB"
**Solution:**
```cmd
# Specify DuckDB path explicitly
cmake .. -G Ninja ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -DDUCKDB_ROOT=C:\duckdb
```

### "Ninja not found"
**Solution:**
```cmd
# Use Visual Studio generator instead
cmake .. -G "Visual Studio 17 2022" -A x64 -T ClangCL
```

### "cl.exe not found" (when using ClangCL)
**Solution:**
- Install Visual Studio Build Tools (free)
- Or use "x64 Native Tools Command Prompt for VS 2022"

### "LINK : fatal error LNK1181"
**Solution:**
```cmd
# Specify DuckDB library path
cmake .. -G Ninja ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -DDUCKDB_LIBRARY=C:\duckdb\libduckdb.dll.lib
```

---

## Quick Reference Commands

```cmd
# 1. Install LLVM (download installer from GitHub)
# 2. Install CMake (download installer from cmake.org)

# 3. Verify
clang --version
cmake --version

# 4. Build
cd C:\Users\prateek\Desktop\spark\pyspark_practice\sql_practice_local\cplusplus
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
ninja

# 5. Run
.\sql-practice-server.exe --port 8080
```

---

## Next Steps After Installation

Once you have Clang installed:

1. **Tell me when installation is complete**
2. **I'll run the build commands for you**
3. **We'll fix any issues that come up**

Let me know when you're ready to proceed!
