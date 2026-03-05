# sync_lib.ps1
# Copies Boring_Project\lib\ into every subdirectory that contains a .ino sketch.
# Run from anywhere: powershell -ExecutionPolicy Bypass -File sync_lib.ps1

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$libSrc    = Join-Path $scriptDir "lib"

if (-not (Test-Path $libSrc -PathType Container)) {
    Write-Error "ERROR: $libSrc does not exist."
    exit 1
}

$found = 0
Get-ChildItem -Path $scriptDir -Directory | ForEach-Object {
    $sketchDir = $_.FullName

    # Only process directories that contain a .ino file
    if (-not (Get-ChildItem -Path $sketchDir -Filter "*.ino" -File)) { return }

    $dest = Join-Path $sketchDir "lib"
    $name = $_.Name

    # Remove existing lib (symlink junction or real directory)
    if (Test-Path $dest) { Remove-Item -Path $dest -Recurse -Force }

    Copy-Item -Path $libSrc -Destination $dest -Recurse
    Write-Host "Synced -> $name\lib"
    $found++
}

Write-Host "$found sketch(es) updated."
