param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Args)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
  return (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

function Resolve-QtPrefix {
  if (-not [string]::IsNullOrWhiteSpace($env:CMAKE_PREFIX_PATH)) {
    $paths = $env:CMAKE_PREFIX_PATH -split ';' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }
    foreach ($path in $paths) {
      if (Test-Path (Join-Path $path "lib\cmake\Qt6\Qt6Config.cmake")) {
        return $path
      }
    }
  }

  if (-not [string]::IsNullOrWhiteSpace($env:QTDIR)) {
    if (Test-Path (Join-Path $env:QTDIR "lib\cmake\Qt6\Qt6Config.cmake")) {
      return $env:QTDIR
    }
  }

  $roots = @("C:\Qt", "$env:USERPROFILE\Qt", "C:\Program Files\Qt")
  foreach ($root in $roots) {
    if (-not (Test-Path $root)) {
      continue
    }

    $candidates = Get-ChildItem -Path $root -Directory -Recurse -ErrorAction SilentlyContinue |
      Where-Object { $_.Name -match "msvc.*64" } |
      Sort-Object FullName -Descending

    foreach ($candidate in $candidates) {
      $configPath = Join-Path $candidate.FullName "lib\cmake\Qt6\Qt6Config.cmake"
      if (Test-Path $configPath) {
        return $candidate.FullName
      }
    }
  }

  return ""
}

function Get-PathMap([string]$RepoRoot) {
  return @{
    DesktopDist = Join-Path $RepoRoot "build\desktop-native\dist\lumos.exe"
    GuiBuildScript = Join-Path $RepoRoot "installer\windows\build_shell_native.ps1"
    CMakeLists = Join-Path $RepoRoot "CMakeLists.txt"
  }
}

function Write-Usage {
  Write-Host "Lumos Launcher"
  Write-Host ""
  Write-Host "Usage:"
  Write-Host "  .\lumos.cmd"
  Write-Host "  .\lumos.cmd start [--force] [args...]"
  Write-Host "  .\lumos.cmd doctor"
  Write-Host "  .\lumos.cmd build [--force]"
  Write-Host ""
  Write-Host "Command-order rule:"
  Write-Host "  Correct: .\lumos.cmd build --force"
  Write-Host "  Wrong:   .\lumos.cmd --force build"
}

function Test-CommandAvailable([string]$CommandName) {
  return $null -ne (Get-Command $CommandName -ErrorAction SilentlyContinue)
}

function Invoke-Doctor([hashtable]$Paths) {
  Write-Host "== Lumos Doctor =="

  if (Test-Path $Paths.CMakeLists) {
    Write-Host "[ok] CMakeLists.txt found"
  } else {
    Write-Host "[warn] CMakeLists.txt missing (desktop build/start commands will fail)"
  }

  if (Test-CommandAvailable "cmake") {
    $cmakeVersion = & cmake --version | Select-Object -First 1
    Write-Host "[ok] $cmakeVersion"
  } else {
    Write-Host "[warn] cmake not found in PATH"
  }

  if (Test-CommandAvailable "git") {
    $gitVersion = & git --version
    Write-Host "[ok] $gitVersion"
  } else {
    Write-Host "[warn] git not found in PATH"
  }

  $qtPathEntries = @()
  if ($env:CMAKE_PREFIX_PATH) {
    $qtPathEntries += $env:CMAKE_PREFIX_PATH -split ';'
  }
  if ($env:QTDIR) {
    $qtPathEntries += $env:QTDIR
  }
  $qtCandidates = $qtPathEntries | Where-Object { $_ -match "(?i)qt" }
  if ($qtCandidates.Count -gt 0) {
    Write-Host "[ok] Qt path hints found:"
    $qtCandidates | Select-Object -Unique | ForEach-Object { Write-Host "     $_" }
  } else {
    Write-Host "[warn] No Qt path hints found in CMAKE_PREFIX_PATH/QTDIR"
  }

  $detectedQtPrefix = Resolve-QtPrefix
  if (-not [string]::IsNullOrWhiteSpace($detectedQtPrefix)) {
    Write-Host "[ok] Auto-detected Qt prefix: $detectedQtPrefix"
  } else {
    Write-Host "[warn] No auto-detected Qt prefix found in common install paths"
  }

  if (Test-Path $Paths.DesktopDist) {
    Write-Host "[ok] Desktop artifact: $($Paths.DesktopDist)"
  } else {
    Write-Host "[info] Desktop artifact missing (build with .\lumos.cmd build --force)"
  }
}

function Split-ForceFlag([string[]]$InputArgs) {
  $force = $false
  $remaining = New-Object System.Collections.Generic.List[string]
  foreach ($arg in $InputArgs) {
    if ($arg -eq "--force") {
      $force = $true
      continue
    }
    [void]$remaining.Add($arg)
  }
  return @{
    Force = $force
    Args = @($remaining)
  }
}

function Invoke-Build([switch]$Force, [hashtable]$Paths) {
  $guiCmd = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $Paths.GuiBuildScript)
  if ($Force) {
    $guiCmd += "-Force"
  }
  & powershell @guiCmd
  if ($LASTEXITCODE -ne 0) { throw "Desktop build failed." }
}

function Ensure-Built([switch]$Force, [hashtable]$Paths) {
  $artifact = $Paths.DesktopDist
  if ($Force -or -not (Test-Path $artifact)) {
    Invoke-Build -Force:$Force -Paths $Paths
  }
  return $artifact
}

function Start-Desktop([string]$DesktopExe, [string[]]$ForwardArgs) {
  $qtPrefix = Resolve-QtPrefix
  $originalPath = $env:PATH

  try {
    if (-not [string]::IsNullOrWhiteSpace($qtPrefix)) {
      $qtBin = Join-Path $qtPrefix "bin"
      if (Test-Path $qtBin) {
        $env:PATH = "$qtBin;$env:PATH"
      }
    }

    & $DesktopExe @ForwardArgs
    return $LASTEXITCODE
  } finally {
    $env:PATH = $originalPath
  }
}

function Assert-CommandOrder([string[]]$AllArgs) {
  if ($AllArgs.Count -gt 0 -and $AllArgs[0].StartsWith("-")) {
    throw "Unknown leading flag '$($AllArgs[0])'. Use command-first syntax, e.g. .\lumos.cmd build --force"
  }
}

$repoRoot = Get-RepoRoot
$paths = Get-PathMap -RepoRoot $repoRoot

if ($Args.Count -eq 0) {
  $desktopExe = Ensure-Built -Paths $paths
  $exitCode = Start-Desktop -DesktopExe $desktopExe -ForwardArgs @()
  exit $exitCode
}

Assert-CommandOrder -AllArgs $Args

$command = $Args[0].ToLowerInvariant()
$tail = if ($Args.Count -gt 1) { @($Args[1..($Args.Count - 1)]) } else { @() }

switch ($command) {
  "doctor" {
    Invoke-Doctor -Paths $paths
    exit 0
  }

  "help" {
    Write-Usage
    exit 0
  }

  "build" {
    $parsed = Split-ForceFlag -InputArgs $tail
    if ($parsed.Args.Count -gt 0) {
      throw "Unknown build argument(s): $($parsed.Args -join ' ')"
    }

    Invoke-Build -Force:([bool]$parsed.Force) -Paths $paths
    exit 0
  }

  "start" {
    $parsed = Split-ForceFlag -InputArgs $tail
    $desktopExe = Ensure-Built -Force:([bool]$parsed.Force) -Paths $paths
    $exitCode = Start-Desktop -DesktopExe $desktopExe -ForwardArgs @($parsed.Args)
    exit $exitCode
  }

  "gui" {
    $parsed = Split-ForceFlag -InputArgs $tail
    $desktopExe = Ensure-Built -Force:([bool]$parsed.Force) -Paths $paths
    $exitCode = Start-Desktop -DesktopExe $desktopExe -ForwardArgs @($parsed.Args)
    exit $exitCode
  }

  "cli" {
    throw "CLI command is not supported. Lumos is desktop-only. Use .\lumos.cmd start"
  }

  default {
    throw "Unknown command '$command'. Run .\lumos.cmd help"
  }
}
