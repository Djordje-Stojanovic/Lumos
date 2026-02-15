param(
  [ValidateSet("gui", "cli")]
  [string]$Kind = "gui",
  [string]$BuildDir = "",
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
  & cmake -S $RepoRoot -B $ResolvedBuildDir -G "Visual Studio 17 2022" -A x64
  if ($LASTEXITCODE -ne 0) {
    throw "cmake configure failed for $ResolvedBuildDir"
  }
}

function Invoke-CMakeBuild([string]$ResolvedBuildDir, [string]$Config, [string]$Kind) {
  $targets = Get-AvailableTargets $ResolvedBuildDir
  if ($Kind -eq "gui" -and ($targets -contains "lumos_app")) {
    & cmake --build $ResolvedBuildDir --config $Config --target lumos_app
  } elseif ($Kind -eq "cli" -and ($targets -contains "lumos_cli")) {
    & cmake --build $ResolvedBuildDir --config $Config --target lumos_cli
  } else {
    & cmake --build $ResolvedBuildDir --config $Config
  }

  if ($LASTEXITCODE -ne 0) {
    throw "cmake build failed for $ResolvedBuildDir"
  }
}

function Find-BuiltExecutable([string]$ResolvedBuildDir, [string]$Config, [string]$Kind) {
  $preferred = @()
  if ($Kind -eq "gui") {
    $preferred = @("lumos_app.exe", "lumos.exe")
  } else {
    $preferred = @("lumos_cli.exe")
  }

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

  $allExe = Get-ChildItem -Path $ResolvedBuildDir -Filter *.exe -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -notmatch "(?i)test|zero_check|all_build|run_tests" }

  if ($Kind -eq "cli") {
    $cliCandidate = $allExe | Where-Object { $_.Name -match "(?i)cli" } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($null -ne $cliCandidate) {
      return $cliCandidate.FullName
    }
    return $null
  }

  $guiCandidate = $allExe | Sort-Object LastWriteTime -Descending | Select-Object -First 1
  if ($null -ne $guiCandidate) {
    return $guiCandidate.FullName
  }

  return $null
}

function Stage-DistExecutable([string]$ResolvedBuildDir, [string]$Config, [string]$Kind) {
  $exePath = Find-BuiltExecutable -ResolvedBuildDir $ResolvedBuildDir -Config $Config -Kind $Kind
  if ([string]::IsNullOrWhiteSpace($exePath)) {
    if ($Kind -eq "cli") {
      throw "Build succeeded but no CLI executable was found. Add a lumos_cli target or provide a CLI binary."
    }
    throw "Build succeeded but no executable was found to stage."
  }

  $distDir = Join-Path $ResolvedBuildDir "dist"
  New-Item -ItemType Directory -Force -Path $distDir | Out-Null

  $distExe = if ($Kind -eq "gui") { Join-Path $distDir "lumos-gui.exe" } else { Join-Path $distDir "lumos-cli.exe" }
  Copy-Item -Force $exePath $distExe

  return $distExe
}

$repoRoot = Get-RepoRoot
Ensure-CMakeLists -RepoRoot $repoRoot

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
  $BuildDir = if ($Kind -eq "gui") { "build\gui-native" } else { "build\cli-native" }
}

$resolvedBuildDir = Join-Path $repoRoot $BuildDir

if ($Force -and (Test-Path $resolvedBuildDir)) {
  Remove-Item -Recurse -Force $resolvedBuildDir
}

Invoke-CMakeConfigure -RepoRoot $repoRoot -ResolvedBuildDir $resolvedBuildDir
Invoke-CMakeBuild -ResolvedBuildDir $resolvedBuildDir -Config $Config -Kind $Kind
$staged = Stage-DistExecutable -ResolvedBuildDir $resolvedBuildDir -Config $Config -Kind $Kind

Write-Host "staged: $staged"

