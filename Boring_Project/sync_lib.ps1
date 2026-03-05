# sync_lib.ps1
# Copies Boring_Project\lib\ and Boring_Project\src\*.cpp into every
# subdirectory that contains a .ino sketch.
# Run from anywhere: powershell -ExecutionPolicy Bypass -File sync_lib.ps1

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$libSrc    = Join-Path $scriptDir "lib"
$srcDir    = Join-Path $scriptDir "src"

if (-not (Test-Path $libSrc -PathType Container)) {
    Write-Error "ERROR: $libSrc does not exist."
    exit 1
}
if (-not (Test-Path $srcDir -PathType Container)) {
    Write-Error "ERROR: $srcDir does not exist."
    exit 1
}

$found = 0
Get-ChildItem -Path $scriptDir -Directory | ForEach-Object {
    $sketchDir = $_.FullName

    # Only process directories that contain a .ino file
    if (-not (Get-ChildItem -Path $sketchDir -Filter "*.ino" -File)) { return }

    $name = $_.Name

    # --- Sync lib/ ---
    $libDest = Join-Path $sketchDir "lib"
    if (Test-Path $libDest) { Remove-Item -Path $libDest -Recurse -Force }
    Copy-Item -Path $libSrc -Destination $libDest -Recurse

    # --- Sync src/*.cpp to sketch root ---
    Get-ChildItem -Path $srcDir -Filter "*.cpp" -File | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $sketchDir -Force
    }

    Write-Host "Synced -> $name\"
    $found++
}

Write-Host "$found sketch(es) updated."
