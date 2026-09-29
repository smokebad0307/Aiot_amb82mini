param(
    [string]$BoardIp = "192.168.50.96",
    [string]$ListenIp = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ListenIp)) {
    $route = Find-NetRoute -RemoteIPAddress $BoardIp -ErrorAction Stop |
        Where-Object { $_.IPAddress -match '^\d+\.\d+\.\d+\.\d+$' -and $_.IPAddress -notlike '169.254.*' } |
        Select-Object -First 1
    if (-not $route) {
        throw "Could not find the local IPv4 route to AMB82 at $BoardIp. Connect this PC to the same Wi-Fi, then retry with -ListenIp."
    }
    $ListenIp = $route.IPAddress
}

$parsedIp = $null
if (-not [System.Net.IPAddress]::TryParse($ListenIp, [ref]$parsedIp) -or $parsedIp.AddressFamily -ne [System.Net.Sockets.AddressFamily]::InterNetwork) {
    throw "ListenIp must be an IPv4 address, for example 192.168.50.197."
}

$certDir = Join-Path $PSScriptRoot "certs"
$pfxPath = Join-Path $certDir "server.pfx"
$rootPath = Join-Path $certDir "ios-root.cer"
$configPath = Join-Path $certDir "config.json"
New-Item -ItemType Directory -Force -Path $certDir | Out-Null

if ((Test-Path -LiteralPath $pfxPath) -or (Test-Path -LiteralPath $rootPath) -or (Test-Path -LiteralPath $configPath)) {
    throw "HTTPS certificate files already exist under https-proxy\certs. Back them up and remove that folder before generating a replacement certificate."
}

$password = Read-Host "Set a password for the HTTPS server certificate (at least 8 characters)" -AsSecureString
if ($password.Length -lt 8) {
    throw "Use a certificate password with at least 8 characters. No certificates were created."
}

$now = Get-Date
$ca = New-SelfSignedCertificate `
    -Type Custom `
    -Subject "CN=AMB82 MINI Local HTTPS Root $env:COMPUTERNAME" `
    -KeyAlgorithm RSA `
    -KeyLength 3072 `
    -HashAlgorithm SHA256 `
    -KeyUsage CertSign, CRLSign, DigitalSignature `
    -KeyUsageProperty Sign `
    -KeyExportPolicy NonExportable `
    -TextExtension @("2.5.29.19={critical}{text}ca=TRUE&pathlength=0") `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -NotBefore $now.AddMinutes(-5) `
    -NotAfter $now.AddYears(5)

$serverCert = New-SelfSignedCertificate `
    -Type Custom `
    -Subject "CN=$ListenIp" `
    -Signer $ca `
    -KeyAlgorithm RSA `
    -KeyLength 2048 `
    -HashAlgorithm SHA256 `
    -KeyUsage DigitalSignature, KeyEncipherment `
    -KeyUsageProperty Sign, Decrypt `
    -KeyExportPolicy Exportable `
    -TextExtension @(
        "2.5.29.17={text}IPAddress=$ListenIp",
        "2.5.29.37={text}1.3.6.1.5.5.7.3.1"
    ) `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -NotBefore $now.AddMinutes(-5) `
    -NotAfter $now.AddYears(1)

Export-PfxCertificate -Cert $serverCert -FilePath $pfxPath -Password $password -ChainOption EndEntityCertOnly -NoProperties | Out-Null
Export-Certificate -Cert $ca -FilePath $rootPath -Type CERT | Out-Null

$config = [ordered]@{
    boardOrigin = "http://$BoardIp"
    listenIp = $ListenIp
    port = 8443
}
[System.IO.File]::WriteAllText($configPath, ($config | ConvertTo-Json), [System.Text.UTF8Encoding]::new($false))

Write-Host "Created HTTPS certificate files in https-proxy\certs."
Write-Host "iPhone URL: https://${ListenIp}:8443/"
Write-Host "Transfer https-proxy\certs\ios-root.cer to your iPhone and trust it before opening that URL."
Write-Host "Keep server.pfx private. It contains the HTTPS server's private key."
