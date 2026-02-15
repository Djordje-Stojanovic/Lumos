param(
  [string]$BuildDir = "build\cli-native",
  [string]$Config = "Release",
  [switch]$Force
)

$ErrorActionPreference = "Stop"

$buildScript = Join-Path $PSScriptRoot "build_native.ps1"
$cmdArgs = @(
  "-NoProfile",
  "-ExecutionPolicy", "Bypass",
  "-File", $buildScript,
  "-Kind", "cli",
  "-BuildDir", $BuildDir,
  "-Config", $Config
)
if ($Force) {
  $cmdArgs += "-Force"
}
& powershell @cmdArgs
exit $LASTEXITCODE
