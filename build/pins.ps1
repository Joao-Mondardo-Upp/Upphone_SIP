# Versoes fixas das dependencias externas do Upphone SIP.
# Para atualizar uma dependencia: troque o commit aqui, rode o build completo e teste antes de publicar.

$Pins = @{
    # pjproject master de 09/10/2026 (2.17-dev): "Support app managed SDP for signalling-plane B2BUA (#5255)"
    pjproject = @{ Url = 'https://github.com/pjsip/pjproject.git';                Commit = 'bd69054a696054db8fb4e62b759a3ecaf26c0189' }
    # bcg729 (G.729) 1.1.x, 14/07/2026
    bcg729    = @{ Url = 'https://github.com/BelledonneCommunications/bcg729.git'; Commit = 'fb195811fe630415c9bf433c0c0bebde14314ff2' }
    # SQLiteCpp (inclui o sqlite3.c), 02/10/2026
    SQLiteCpp = @{ Url = 'https://github.com/SRombauts/SQLiteCpp.git';            Commit = 'd07876c3090fa57f92af9e7fc92bb4b3f3c27390' }
}

function Invoke-Git {
    # git escreve progresso no stderr; no PowerShell 5.1 isso nao pode virar erro
    $prev = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $output = & git @args 2>&1
    $code = $LASTEXITCODE
    $ErrorActionPreference = $prev
    if ($code -ne 0) { throw "git $($args -join ' ') falhou:`n$output" }
    return ($output | Where-Object { $_ -is [string] })
}

function Get-GitHead([string]$Dir) {
    if (-not (Test-Path (Join-Path $Dir '.git'))) { return $null }
    try { return (Invoke-Git -C $Dir rev-parse HEAD | Select-Object -First 1) } catch { return $null }
}

# Garante que $Dir e um clone de $Url exatamente no commit $Commit (baixa so esse commit).
function Sync-PinnedRepo([string]$Url, [string]$Dir, [string]$Commit) {
    if (-not (Test-Path (Join-Path $Dir '.git'))) {
        if ((Test-Path $Dir) -and (Get-ChildItem $Dir -Force | Select-Object -First 1)) {
            throw "$Dir ja existe e nao e um repositorio git"
        }
        New-Item -ItemType Directory -Force $Dir | Out-Null
        Invoke-Git -C $Dir init -q | Out-Null
        Invoke-Git -C $Dir remote add origin $Url | Out-Null
    }
    if ((Get-GitHead $Dir) -ne $Commit) {
        Write-Host "    $Url @ $($Commit.Substring(0, 10))"
        Invoke-Git -C $Dir fetch -q --depth 1 origin $Commit | Out-Null
        Invoke-Git -C $Dir checkout -q --detach FETCH_HEAD | Out-Null
    }
}
