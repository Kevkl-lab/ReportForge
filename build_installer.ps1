# build_installer.ps1
# This script builds the ReportForge application and packages it into a Windows NSIS installer.

param (
    [string]$QtDir = "" # Optional: Override the auto-detected Qt path
)

# Enable error action preference
$ErrorActionPreference = "Stop"

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host " ReportForge Windows Installer Build Script  " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

# 1. Detect Qt Directory if not provided
if (-not $QtDir) {
    Write-Host "Searching for local Qt 6 MSVC installation under C:\Qt..."
    if (Test-Path "C:\Qt") {
        $qtMatches = Get-ChildItem -Path "C:\Qt" -Filter "6.*" | ForEach-Object {
            $msvcPath = Join-Path $_.FullName "msvc2022_64"
            if (Test-Path $msvcPath) { $msvcPath }
        }
        if ($qtMatches) {
            # Pick the latest version
            $QtDir = $qtMatches | Sort-Object | Select-Object -Last 1
            Write-Host "Auto-detected Qt at: $QtDir" -ForegroundColor Green
        }
    }
}

if (-not $QtDir -or -not (Test-Path $QtDir)) {
    Write-Host ""
    Write-Host "ERROR: Qt installation directory not found!" -ForegroundColor Red
    Write-Host "Please install Qt 6 (MSVC 2022 64-bit) or specify the path manually, e.g.:" -ForegroundColor Yellow
    Write-Host "  .\build_installer.ps1 -QtDir 'C:\Qt\6.8.0\msvc2022_64'" -ForegroundColor Yellow
    exit 1
}

# 2. Check for CMake
if (-not (Get-Command "cmake" -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: CMake was not found in your PATH." -ForegroundColor Red
    Write-Host "Please install CMake from https://cmake.org/download/ and ensure it is added to your environment variables." -ForegroundColor Yellow
    exit 1
}

# 3. Check and set up NSIS (makensis)
$nsisDefaultPath = "${env:ProgramFiles(x86)}\NSIS"
if (-not (Get-Command "makensis" -ErrorAction SilentlyContinue)) {
    if (Test-Path "$nsisDefaultPath\makensis.exe") {
        Write-Host "Found NSIS in default folder, adding to temporary PATH..." -ForegroundColor Green
        $env:PATH = "$env:PATH;$nsisDefaultPath"
    } else {
        Write-Host "WARNING: NSIS installer compiler (makensis.exe) not found in PATH or standard location." -ForegroundColor Yellow
        Write-Host "Please install NSIS from https://nsis.sourceforge.io/ so CPack can generate the installer." -ForegroundColor Yellow
    }
}

# 4. Clean and recreate build directory
$buildDir = "build"
if (Test-Path $buildDir) {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Gray
    Remove-Item -Recurse -Force $buildDir
}
New-Item -ItemType Directory -Force -Path $buildDir > $null

# 5. Configure CMake
Write-Host "Configuring CMake project (Release mode)..." -ForegroundColor Cyan
cmake -B $buildDir -S . -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$QtDir" "-DCMAKE_BUILD_TYPE=Release"
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed." -ForegroundColor Red
    exit 1
}

# 6. Build the application
Write-Host "Compiling ReportForge..." -ForegroundColor Cyan
cmake --build $buildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Compilation failed." -ForegroundColor Red
    exit 1
}

# 7. Package the installer using CPack
Write-Host "Packaging installer with CPack..." -ForegroundColor Cyan
cpack --config "$buildDir/CPackConfig.cmake"
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CPack packaging failed." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "=============================================" -ForegroundColor Green
Write-Host " SUCCESS! Windows installer built successfully." -ForegroundColor Green
Write-Host " You can find the installer .exe in the root directory." -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
