$ErrorActionPreference = "Stop"

$certDir = Join-Path $PSScriptRoot "certs"
$configPath = Join-Path $certDir "config.json"
$pfxPath = Join-Path $certDir "server.pfx"
if (-not (Test-Path -LiteralPath $configPath) -or -not (Test-Path -LiteralPath $pfxPath)) {
    throw "Run .\https-proxy\create-cert.ps1 first."
}

$node = Get-Command node -ErrorAction SilentlyContinue
if (-not $node) {
    throw "Node.js is required to run the HTTPS proxy. Install Node.js LTS, reopen PowerShell, then retry."
}

$config = Get-Content -Raw -LiteralPath $configPath | ConvertFrom-Json
$securePassword = Read-Host "Enter the HTTPS certificate password" -AsSecureString
$passwordPtr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($securePassword)
try {
    $env:TLS_PFX_PASSWORD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($passwordPtr)
    & $node.Source (Join-Path $PSScriptRoot "server.mjs")
    if ($LASTEXITCODE -ne 0) { throw "HTTPS proxy exited with code $LASTEXITCODE." }
}
finally {
    [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($passwordPtr)
    Remove-Item Env:\TLS_PFX_PASSWORD -ErrorAction SilentlyContinue
}
