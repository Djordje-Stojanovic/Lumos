param(
  [string]$BuildDir = "build\desktop-native",
  [string]$Config = "Release",
  [switch]$Force
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
  return (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

function Ensure-CMakeLists([string]$RepoRoot) {
  $cmakeLists = Join-Path $RepoRoot "CMakeLists.txt"
  if (-not (Test-Path $cmakeLists)) {
    throw "Missing CMakeLists.txt at repo root. Merge/create the CMake project first."
  }
}

function Get-TargetDirectoriesFile([string]$ResolvedBuildDir) {
  return Join-Path $ResolvedBuildDir "CMakeFiles\TargetDirectories.txt"
}

function Get-AvailableTargets([string]$ResolvedBuildDir) {
  $targetsFile = Get-TargetDirectoriesFile $ResolvedBuildDir
  if (-not (Test-Path $targetsFile)) {
    return @()
  }

  $targets = New-Object System.Collections.Generic.List[string]
  Get-Content $targetsFile | ForEach-Object {
    if ($_ -match "CMakeFiles[\\/](.+?)\.dir") {
      [void]$targets.Add($Matches[1])
    }
  }
  return $targets | Sort-Object -Unique
}

function Invoke-CMakeConfigure([string]$RepoRoot, [string]$ResolvedBuildDir) {
  & cmake -S $RepoRoot -B $ResolvedBuildDir -G "Visual Studio 17 2022" -A x64 -DLUMOS_BUILD_UI=ON
  if ($LASTEXITCODE -ne 0) {
    throw "cmake configure failed for $ResolvedBuildDir"
  }
}

function Assert-GuiTargetAvailable([string]$ResolvedBuildDir) {
  $targets = Get-AvailableTargets $ResolvedBuildDir
  if (-not ($targets -contains "lumos_app")) {
    throw "Desktop target 'lumos_app' was not generated. Ensure Qt6 is installed and discoverable (set CMAKE_PREFIX_PATH or QTDIR)."
  }
}

function Invoke-CMakeBuild([string]$ResolvedBuildDir, [string]$Config) {
  & cmake --build $ResolvedBuildDir --config $Config --target lumos_app
  if ($LASTEXITCODE -ne 0) {
    throw "cmake build failed for $ResolvedBuildDir"
  }
}

function Find-BuiltExecutable([string]$ResolvedBuildDir, [string]$Config) {
  $preferred = @("lumos_app.exe", "lumos.exe")
  $searchRoots = @(
    (Join-Path $ResolvedBuildDir $Config),
    $ResolvedBuildDir
  )

  foreach ($root in $searchRoots) {
    foreach ($name in $preferred) {
      $candidate = Join-Path $root $name
      if (Test-Path $candidate) {
        return $candidate
      }
    }
  }
  return $null
}

function Stage-DistExecutable([string]$ResolvedBuildDir, [string]$Config) {
  $exePath = Find-BuiltExecutable -ResolvedBuildDir $ResolvedBuildDir -Config $Config
  if ([string]::IsNullOrWhiteSpace($exePath)) {
    throw "Build succeeded but no lumos_app executable was found to stage."
  }

  $distDir = Join-Path $ResolvedBuildDir "dist"
  New-Item -ItemType Directory -Force -Path $distDir | Out-Null

  $distExe = Join-Path $distDir "lumos.exe"
  Copy-Item -Force $exePath $distExe

  return $distExe
}

function Try-DeployQtRuntime([string]$RepoRoot, [string]$DistExePath) {
  $windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
  if ($null -eq $windeployqt) {
    Write-Host "[warn] windeployqt not found; runtime DLL deployment skipped."
    return
  }

  $qmlDir = Join-Path $RepoRoot "src\ui\qml"
  if (Test-Path $qmlDir) {
    & $windeployqt.Source --no-translations --qmldir $qmlDir $DistExePath
  } else {
    & $windeployqt.Source --no-translations $DistExePath
  }

  if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed for $DistExePath"
  }
}

$repoRoot = Get-RepoRoot
Ensure-CMakeLists -RepoRoot $repoRoot
$resolvedBuildDir = Join-Path $repoRoot $BuildDir

if ($Force -and (Test-Path $resolvedBuildDir)) {
  Remove-Item -Recurse -Force $resolvedBuildDir
}

Invoke-CMakeConfigure -RepoRoot $repoRoot -ResolvedBuildDir $resolvedBuildDir
Assert-GuiTargetAvailable -ResolvedBuildDir $resolvedBuildDir
Invoke-CMakeBuild -ResolvedBuildDir $resolvedBuildDir -Config $Config
$staged = Stage-DistExecutable -ResolvedBuildDir $resolvedBuildDir -Config $Config
Try-DeployQtRuntime -RepoRoot $repoRoot -DistExePath $staged

Write-Host "staged: $staged"

