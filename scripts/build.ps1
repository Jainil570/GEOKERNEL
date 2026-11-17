# Configure, build and test on Windows.
#   .\scripts\build.ps1            # Visual Studio 2022 (msvc preset)
#   .\scripts\build.ps1 -Preset dev  # Ninja (run from a "Developer PowerShell")
param(
    [string]$Preset = "msvc"
)
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build --preset $Preset --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

ctest --preset $Preset
exit $LASTEXITCODE
