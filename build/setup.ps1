<#
.SYNOPSIS
    Monta do zero a estrutura de build do Upphone SIP, com as versoes fixadas em pins.ps1.

.DESCRIPTION
    Cria <Dir> com o pjproject no commit fixado e clona este repositorio (no mesmo commit
    em que este script esta) em <Dir>\microsip. Depois basta rodar o build.

.EXAMPLE
    git clone https://github.com/Joao-Mondardo-Upp/Upphone_SIP.git
    .\Upphone_SIP\build\setup.ps1 -Dir C:\src\upphone
    C:\src\upphone\microsip\build\build.ps1 -Installer
#>
param(
    [Parameter(Mandatory = $true)]
    [string]$Dir
)

$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'pins.ps1')

$Dir = [IO.Path]::GetFullPath($Dir)
if ($Dir.Length -gt 60) {
    Write-Warning "Caminho longo ($($Dir.Length) caracteres). O build do pjproject pode passar do limite de 260 caracteres do Windows; prefira algo como C:\src\upphone."
}

Write-Host "==> pjproject"
Sync-PinnedRepo $Pins.pjproject.Url $Dir $Pins.pjproject.Commit

$repoUrl = Invoke-Git -C $PSScriptRoot remote get-url origin | Select-Object -First 1
$repoCommit = Invoke-Git -C $PSScriptRoot rev-parse HEAD | Select-Object -First 1
$appDir = Join-Path $Dir 'microsip'
Write-Host "==> Upphone SIP ($repoUrl @ $($repoCommit.Substring(0, 10)))"
if (-not (Test-Path (Join-Path $appDir '.git'))) {
    Invoke-Git clone -q $repoUrl $appDir | Out-Null
}
Invoke-Git -C $appDir fetch -q origin | Out-Null
Invoke-Git -C $appDir checkout -q $repoCommit | Out-Null

Write-Host "`nPronto. Para compilar:"
Write-Host "  $appDir\build\build.ps1 -Installer"
