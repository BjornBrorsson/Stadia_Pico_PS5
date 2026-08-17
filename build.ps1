# PowerShell build script for Stadia to PS5/Wingman Bridge on Raspberry Pi Pico WH

param(
    [string]$BuildType = "Release",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  Stadia to PS5/Wingman Bridge - Raspberry Pi Pico WH Build" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# Check if PICO_SDK_PATH is set, or default to standard locations
if (-not $env:PICO_SDK_PATH) {
    if (Test-Path "$env:USERPROFILE\pico-sdk") {
        $env:PICO_SDK_PATH = "$env:USERPROFILE\pico-sdk"
        Write-Host "Found Pico SDK at: $env:PICO_SDK_PATH" -ForegroundColor Green
    } elseif (Test-Path "$env:USERPROFILE\.pico-sdk\sdk\2.0.0") {
        $env:PICO_SDK_PATH = "$env:USERPROFILE\.pico-sdk\sdk\2.0.0"
        Write-Host "Found Pico SDK at: $env:PICO_SDK_PATH" -ForegroundColor Green
    } else {
        Write-Host "NOTE: PICO_SDK_PATH is not set. CMake will attempt to fetch it automatically via git." -ForegroundColor Yellow
        $env:PICO_SDK_FETCH_FROM_GIT = "on"
    }
}

if ($Clean -and (Test-Path "build")) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
}

if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Write-Host "Configuring CMake project..." -ForegroundColor Cyan
cmake -B build -DCMAKE_BUILD_TYPE=$BuildType -DPICO_BOARD=pico_w

Write-Host "Building firmware..." -ForegroundColor Cyan
cmake --build build --config $BuildType -j

if (Test-Path "build\stadia_ps5_bridge.uf2") {
    Write-Host "`nBuild SUCCESSFUL!" -ForegroundColor Green
    Write-Host "UF2 Firmware File: $(Resolve-Path build\stadia_ps5_bridge.uf2)" -ForegroundColor Green
    Write-Host "`nTo Flash your Pico WH:" -ForegroundColor White
    Write-Host "1. Hold the BOOTSEL button on the Pico WH while plugging into USB." -ForegroundColor Gray
    Write-Host "2. Drag and drop 'build\stadia_ps5_bridge.uf2' onto the RPI-RP2 drive." -ForegroundColor Gray
} else {
    Write-Host "`nBuild complete. Check build directory for output artifacts." -ForegroundColor Yellow
}
