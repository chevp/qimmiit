# vcpkg Setup Guide

This project uses vcpkg for dependency management. Here's how to set it up for both local development and CI/CD.

---

## Local Setup (Windows)

### Step 1: Set VCPKG_ROOT Environment Variable

You need to tell CMake where your vcpkg installation is located.

**Option A: System-wide (Recommended)**

1. Open **System Environment Variables**:
   - Press `Win + R`
   - Type `sysdm.cpl` and press Enter
   - Click **Advanced** → **Environment Variables**

2. Add new **User Variable** (or System Variable):
   - Variable name: `VCPKG_ROOT`
   - Variable value: `C:\vcpkg`

3. Click **OK** and restart your terminal/IDE

**Option B: Per-session (Temporary)**

In PowerShell:
```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
```

In CMD:
```cmd
set VCPKG_ROOT=C:\vcpkg
```

### Step 2: Verify Setup

```powershell
echo $env:VCPKG_ROOT
# Should output: C:\vcpkg
```

### Step 3: Build the Project

```bash
cmake --preset default
cmake --build build --config Debug
```

CMake will:
1. Read `vcpkg.json` to see what packages you need
2. Use `$env{VCPKG_ROOT}` to find vcpkg toolchain
3. Install missing packages automatically
4. Configure and build your project

---

## How It Works

```
┌─────────────────────────────────────────────────────┐
│  vcpkg.json (in your project)                       │
│  - Lists dependencies: vulkan, glfw3, protobuf...   │
└─────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────┐
│  CMakePresets.json                                  │
│  - Uses: $env{VCPKG_ROOT}/scripts/.../vcpkg.cmake   │
└─────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────┐
│  $VCPKG_ROOT (e.g., C:\vcpkg)                       │
│  - Reads vcpkg.json                                 │
│  - Downloads & builds packages                      │
│  - Installs to C:\vcpkg\installed\x64-windows\      │
└─────────────────────────────────────────────────────┘
```

---

## GitHub Actions / CI Setup

Create `.github/workflows/build.yml`:

```yaml
name: Build Qimmiit Engine

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build-windows:
    runs-on: windows-latest

    steps:
    - name: Checkout code
      uses: actions/checkout@v3

    - name: Setup vcpkg
      uses: microsoft/setup-vcpkg@v1
      with:
        vcpkg-version: 'latest'

    - name: Install Vulkan SDK
      uses: humbletim/setup-vulkan-sdk@v1.2.0
      with:
        vulkan-query-version: latest
        vulkan-use-cache: true

    # vcpkg will automatically read vcpkg.json
    - name: Cache vcpkg packages
      uses: actions/cache@v3
      with:
        path: |
          ${{ env.VCPKG_ROOT }}/installed
          ${{ env.VCPKG_ROOT }}/packages
        key: vcpkg-${{ runner.os }}-${{ hashFiles('vcpkg.json') }}
        restore-keys: |
          vcpkg-${{ runner.os }}-

    - name: Configure CMake
      run: |
        cmake --preset default

    - name: Build
      run: |
        cmake --build build --config Debug

    - name: Upload artifacts
      uses: actions/upload-artifact@v3
      with:
        name: qimmiit-windows-x64
        path: |
          build/bin/Debug/nunaq.exe
          build/bin/Debug/nanook-cli.exe

  build-linux:
    runs-on: ubuntu-latest

    steps:
    - name: Checkout code
      uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          vulkan-tools \
          libvulkan-dev \
          vulkan-validationlayers-dev \
          spirv-tools \
          libglfw3-dev \
          libglm-dev

    - name: Setup vcpkg
      uses: microsoft/setup-vcpkg@v1
      with:
        vcpkg-version: 'latest'

    - name: Configure CMake
      run: |
        mkdir build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Debug \
                 -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

    - name: Build
      run: |
        cmake --build build -j$(nproc)
```

---

## Why Use vcpkg.json?

### ✅ **Keep vcpkg.json** because:

1. **Declares Dependencies** - Lists exactly what your project needs
2. **Version Control** - Track dependency versions in git
3. **Reproducible Builds** - Anyone can build with same dependencies
4. **CI/CD Integration** - GitHub Actions reads it automatically
5. **Cross-Platform** - Works on Windows, Linux, macOS
6. **Team Collaboration** - Everyone gets the same packages

### ❌ **Don't delete vcpkg.json!**

Without it:
- vcpkg doesn't know what packages to install
- Other developers won't know the dependencies
- CI builds will fail
- Manual package installation required

---

## Workflow Comparison

### Local Development

```bash
# You (on Windows with C:\vcpkg)
$env:VCPKG_ROOT = "C:\vcpkg"
cmake --preset default
cmake --build build --config Debug
```

### CI (GitHub Actions)

```yaml
# CI automatically:
# 1. Installs vcpkg to $VCPKG_ROOT
# 2. Reads vcpkg.json
# 3. Installs packages
# 4. Builds project
```

Both use the **same vcpkg.json**, but different vcpkg installations!

---

## Troubleshooting

### Issue: "VCPKG_ROOT not set"

**Solution:**
```powershell
# PowerShell
$env:VCPKG_ROOT = "C:\vcpkg"

# Or add to Windows Environment Variables permanently
```

### Issue: "vcpkg.cmake not found"

**Solution:**
Check that vcpkg is installed:
```bash
dir C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

If missing, clone vcpkg:
```bash
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

### Issue: Packages not installing

**Solution:**
Force vcpkg to reinstall:
```bash
cd C:\vcpkg
.\vcpkg remove --outdated
.\vcpkg integrate install
```

Then rebuild:
```bash
cmake --preset default
```

---

## Best Practices

### ✅ DO:
- Keep `vcpkg.json` in version control
- Set `VCPKG_ROOT` environment variable
- Use `CMakePresets.json` with `$env{VCPKG_ROOT}`
- Cache vcpkg packages in CI

### ❌ DON'T:
- Hardcode absolute paths in `CMakePresets.json`
- Delete `vcpkg.json`
- Commit `vcpkg_installed/` to git
- Commit built packages to git

---

## Summary

| File | Purpose | Keep in Git? |
|------|---------|--------------|
| `vcpkg.json` | Declare dependencies | ✅ YES |
| `CMakePresets.json` | CMake configuration | ✅ YES |
| `C:\vcpkg/` | vcpkg installation | ❌ NO (local only) |
| `vcpkg_installed/` | Installed packages | ❌ NO (add to .gitignore) |

**The key insight:**
- `vcpkg.json` = WHAT you need (portable, in git)
- `$VCPKG_ROOT` = WHERE vcpkg is (local, not in git)
- They work together! 🎯
