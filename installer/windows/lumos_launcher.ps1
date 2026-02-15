param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Args)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
  return (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
}

function Get-PathMap([string]$RepoRoot) {
  return @{
    GuiDist = Join-Path $RepoRoot "build\gui-native\dist\lumos-gui.exe"
    CliDist = Join-Path $RepoRoot "build\cli-native\dist\lumos-cli.exe"
    GuiBuildScript = Join-Path $RepoRoot "installer\windows\build_shell_native.ps1"
    CliBuildScript = Join-Path $RepoRoot "installer\windows\build_platform_native.ps1"
    CMakeLists = Join-Path $RepoRoot "CMakeLists.txt"
  }
}

function Write-Usage {
  Write-Host "Lumos Launcher"
  Write-Host ""
  Write-Host "Usage:"
  Write-Host "  .\lumos.cmd"
  Write-Host "  .\lumos.cmd gui [--force] [args...]"
  Write-Host "  .\lumos.cmd cli [--force] [args...]"
  Write-Host "  .\lumos.cmd doctor"
  Write-Host "  .\lumos.cmd build [all|gui|cli] [--force]"
  Write-Host ""
  Write-Host "Command-order rule:"
  Write-Host "  Correct: .\lumos.cmd build all --force"
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
    Write-Host "[warn] CMakeLists.txt missing (build commands will fail until project scaffold is merged)"
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

  if (Test-Path $Paths.GuiDist) {
    Write-Host "[ok] GUI artifact: $($Paths.GuiDist)"
  } else {
    Write-Host "[info] GUI artifact missing (build with .\lumos.cmd build gui)"
  }

  if (Test-Path $Paths.CliDist) {
    Write-Host "[ok] CLI artifact: $($Paths.CliDist)"
  } else {
    Write-Host "[info] CLI artifact missing (build with .\lumos.cmd build cli)"
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

function Invoke-Build([string]$BuildKind, [switch]$Force, [hashtable]$Paths) {
  $guiCmd = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $Paths.GuiBuildScript)
  if ($Force) {
    $guiCmd += "-Force"
  }

  $cliCmd = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $Paths.CliBuildScript)
  if ($Force) {
    $cliCmd += "-Force"
  }

  switch ($BuildKind) {
    "gui" {
      & powershell @guiCmd
      if ($LASTEXITCODE -ne 0) { throw "GUI build failed." }
    }
    "cli" {
      & powershell @cliCmd
      if ($LASTEXITCODE -ne 0) { throw "CLI build failed." }
    }
    "all" {
      & powershell @guiCmd
      if ($LASTEXITCODE -ne 0) { throw "GUI build failed as part of build all." }

      & powershell @cliCmd
      if ($LASTEXITCODE -ne 0) { throw "CLI build failed as part of build all." }
    }
    default {
      throw "Unknown build target '$BuildKind'. Use: all, gui, or cli."
    }
  }
}

function Ensure-Built([string]$Kind, [switch]$Force, [hashtable]$Paths) {
  $artifact = if ($Kind -eq "gui") { $Paths.GuiDist } else { $Paths.CliDist }
  if ($Force -or -not (Test-Path $artifact)) {
    Invoke-Build -BuildKind $Kind -Force:$Force -Paths $Paths
  }
  return $artifact
}

function Assert-CommandOrder([string[]]$AllArgs) {
  if ($AllArgs.Count -gt 0 -and $AllArgs[0].StartsWith("-")) {
    throw "Unknown leading flag '$($AllArgs[0])'. Use command-first syntax, e.g. .\lumos.cmd build all --force"
  }
}

$repoRoot = Get-RepoRoot
$paths = Get-PathMap -RepoRoot $repoRoot

if ($Args.Count -eq 0) {
  $guiExe = Ensure-Built -Kind "gui" -Paths $paths
  & $guiExe
  exit $LASTEXITCODE
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
    $buildKind = "all"
    $buildArgs = $tail

    if ($buildArgs.Count -gt 0 -and @("all", "gui", "cli") -contains $buildArgs[0].ToLowerInvariant()) {
      $buildKind = $buildArgs[0].ToLowerInvariant()
      $buildArgs = if ($buildArgs.Count -gt 1) { @($buildArgs[1..($buildArgs.Count - 1)]) } else { @() }
    }

    $parsed = Split-ForceFlag -InputArgs $buildArgs
    if ($parsed.Args.Count -gt 0) {
      throw "Unknown build argument(s): $($parsed.Args -join ' ')"
    }

    Invoke-Build -BuildKind $buildKind -Force:([bool]$parsed.Force) -Paths $paths
    exit 0
  }

  "gui" {
    $parsed = Split-ForceFlag -InputArgs $tail
    $guiExe = Ensure-Built -Kind "gui" -Force:([bool]$parsed.Force) -Paths $paths
    & $guiExe @($parsed.Args)
    exit $LASTEXITCODE
  }

  "cli" {
    $parsed = Split-ForceFlag -InputArgs $tail
    $cliExe = Ensure-Built -Kind "cli" -Force:([bool]$parsed.Force) -Paths $paths
    & $cliExe @($parsed.Args)
    exit $LASTEXITCODE
  }

  default {
    $guiExe = Ensure-Built -Kind "gui" -Paths $paths
    & $guiExe @Args
    exit $LASTEXITCODE
  }
}
