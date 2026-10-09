<#
.SYNOPSIS
    Publica uma versao do Upphone SIP no GitHub Releases.

.DESCRIPTION
    Usa os arquivos gerados por "build.ps1 -Installer" em dist\<versao>\ e:
      1. confere que o codigo esta commitado e enviado (git push) e que os .exe sao da versao certa;
      2. monta os ZIPs portateis e o SHA256SUMS.txt;
      3. cria a release e a tag v<versao> no commit atual (HEAD);
      4. envia os arquivos (os que ja estiverem na release sao pulados, entao pode rodar de novo).

    Notas da release: build\notas\v<versao>.md (copie a da versao anterior e ajuste).

    Autenticacao: variavel de ambiente GITHUB_TOKEN, se existir; senao, a credencial que o
    git ja usa para o github.com (Gerenciador de Credenciais do Windows). Precisa de permissao
    de escrita no repositorio.

.EXAMPLE
    .\release.ps1 -DryRun     # so confere, nao publica nada
    .\release.ps1             # publica a versao do const.h
#>
param(
    [string]$Version,
    [string]$NotesFile,
    # cria a release como rascunho (visivel so para quem tem acesso de escrita)
    [switch]$Draft,
    # confere tudo e mostra o que faria, sem criar nem enviar nada
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
. (Join-Path $PSScriptRoot 'pins.ps1')

$AppDir = Split-Path -Parent $PSScriptRoot
function Step($text) { Write-Host "`n==> $text" -ForegroundColor Cyan }

# ---------------------------------------------------------------- versao e arquivos
if (-not $Version) {
    $Version = ([regex]::Match((Get-Content (Join-Path $AppDir 'const.h') -Raw), '_GLOBAL_VERSION "([^"]+)"')).Groups[1].Value
}
$tag = "v$Version"
$distDir = Join-Path $AppDir "dist\$Version"
if (-not $NotesFile) { $NotesFile = Join-Path $PSScriptRoot "notas\$tag.md" }

Step "Conferindo $tag"
if (-not (Test-Path $NotesFile)) {
    throw "Faltam as notas da release: $NotesFile (copie de build\notas\ da versao anterior e ajuste)"
}
$files = @{}
foreach ($p in 'x64', 'Win32') {
    $exe = Join-Path $distDir "$p\Upphone.exe"
    $setup = Join-Path $distDir "Upphone-SIP-$Version-$p-setup.exe"
    foreach ($f in $exe, $setup) {
        if (-not (Test-Path $f)) { throw "Arquivo nao encontrado: $f (rode build.ps1 -Installer)" }
    }
    $exeVersion = (Get-Item $exe).VersionInfo.ProductVersion
    if ($exeVersion -ne $Version) { throw "$exe e da versao $exeVersion, esperado $Version (recompile)" }
    $files[$p] = @{ Exe = $exe; Setup = $setup }
}

# ---------------------------------------------------------------- git
$dirty = Invoke-Git -C $AppDir status --porcelain
if ($dirty) { throw "Ha alteracoes nao commitadas:`n$($dirty -join "`n")" }
$commit = Invoke-Git -C $AppDir rev-parse HEAD | Select-Object -First 1
Invoke-Git -C $AppDir fetch -q origin | Out-Null
$branch = Invoke-Git -C $AppDir rev-parse --abbrev-ref HEAD | Select-Object -First 1
$ahead = Invoke-Git -C $AppDir rev-list --count "origin/$branch..HEAD" | Select-Object -First 1
if ([int]$ahead -gt 0) { throw "Ha $ahead commit(s) que ainda nao foram para o GitHub. Rode 'git push' antes." }
$origin = Invoke-Git -C $AppDir remote get-url origin | Select-Object -First 1
$repo = ([regex]::Match($origin, 'github\.com[:/](.+?)(\.git)?$')).Groups[1].Value
if (-not $repo) { throw "Nao consegui identificar o repositorio do GitHub em '$origin'" }
Write-Host "Repositorio : $repo"
Write-Host "Commit      : $commit ($branch)"
Write-Host "Notas       : $NotesFile"

# ---------------------------------------------------------------- GitHub
$token = $env:GITHUB_TOKEN
if (-not $token) {
    # o PowerShell 5.1 altera texto enviado por pipe a programas nativos; usa um arquivo
    $credIn = [IO.Path]::GetTempFileName()
    try {
        [IO.File]::WriteAllText($credIn, "protocol=https`nhost=github.com`n`n", (New-Object Text.UTF8Encoding $false))
        $cred = cmd /c "git credential fill < `"$credIn`""
    }
    finally { Remove-Item $credIn -Force -ErrorAction SilentlyContinue }
    $token = ($cred | Where-Object { $_ -like 'password=*' }) -replace '^password=', ''
}
if (-not $token) { throw 'Sem credencial do GitHub: faca um git push uma vez (para o git guardar o login) ou defina GITHUB_TOKEN.' }
$headers = @{ Authorization = "Bearer $token"; Accept = 'application/vnd.github+json'; 'X-GitHub-Api-Version' = '2022-11-28' }
$api = "https://api.github.com/repos/$repo"

$release = $null
try { $release = Invoke-RestMethod -Uri "$api/releases/tags/$tag" -Headers $headers } catch { }
if (-not $release -and $Draft) {
    # rascunhos nao aparecem em /releases/tags
    $release = Invoke-RestMethod -Uri "$api/releases?per_page=30" -Headers $headers | Where-Object { $_.tag_name -eq $tag } | Select-Object -First 1
}
$remoteTag = Invoke-Git ls-remote --tags $origin "refs/tags/$tag" | Select-Object -First 1
$tagCommit = if ($remoteTag) { ($remoteTag -split "\s+")[0] } else { $null }
if ($tagCommit -and $tagCommit -ne $commit) {
    if ($release) {
        Write-Warning "A release $tag foi criada no commit $tagCommit e o atual e $commit. So os arquivos que faltam serao enviados; confira se eles foram compilados do commit da tag."
    }
    else {
        throw "A tag $tag ja existe no GitHub em outro commit ($tagCommit). Use outra versao."
    }
}

# ---------------------------------------------------------------- pacote
Step 'Montando arquivos'
$out = Join-Path $distDir 'release'
New-Item -ItemType Directory -Force $out | Out-Null
$assets = @()
foreach ($p in 'x64', 'Win32') {
    Copy-Item $files[$p].Setup $out -Force
    $assets += Join-Path $out (Split-Path $files[$p].Setup -Leaf)
}
foreach ($p in 'x64', 'Win32') {
    $zip = Join-Path $out "Upphone-SIP-$Version-$p-portable.zip"
    if (Test-Path $zip) { Remove-Item $zip -Force }
    Compress-Archive -Path $files[$p].Exe -DestinationPath $zip
    $assets += $zip
}
$sums = Join-Path $out 'SHA256SUMS.txt'
($assets | ForEach-Object { "{0}  {1}" -f (Get-FileHash $_ -Algorithm SHA256).Hash.ToLower(), (Split-Path $_ -Leaf) }) |
    Set-Content $sums -Encoding ASCII
$assets += $sums
$assets | ForEach-Object { Write-Host ("  {0,-42} {1,10:N0} bytes" -f (Split-Path $_ -Leaf), (Get-Item $_).Length) }

$existing = @()
if ($release) { $existing = @($release.assets | ForEach-Object { $_.name }) }

if ($DryRun) {
    Step 'Simulacao (-DryRun): nada foi publicado'
    if ($release) { Write-Host "A release $tag ja existe: $($release.html_url)" }
    else { Write-Host "Criaria a release $tag no commit $commit$(if ($Draft) { ' (rascunho)' })" }
    foreach ($a in $assets) {
        $name = Split-Path $a -Leaf
        Write-Host ("  {0} {1}" -f $(if ($existing -contains $name) { 'ja na release:' } else { 'enviaria:     ' }), $name)
    }
    return
}

# ---------------------------------------------------------------- publicar
if ($release) {
    Step "Release $tag ja existe, enviando o que falta"
}
else {
    Step "Criando a release $tag"
    $body = @{
        tag_name         = $tag
        target_commitish = $commit
        name             = "Upphone SIP $Version"
        body             = [string][IO.File]::ReadAllText($NotesFile, [Text.Encoding]::UTF8)
        draft            = [bool]$Draft
        prerelease       = $false
    } | ConvertTo-Json
    $release = Invoke-RestMethod -Method Post -Uri "$api/releases" -Headers $headers `
        -Body ([Text.Encoding]::UTF8.GetBytes($body)) -ContentType 'application/json; charset=utf-8'
}

$uploadBase = $release.upload_url -replace '\{.*\}$', ''
foreach ($a in $assets) {
    $name = Split-Path $a -Leaf
    if ($existing -contains $name) { Write-Host "  ja na release: $name"; continue }
    $type = if ($name -like '*.txt') { 'text/plain' } elseif ($name -like '*.zip') { 'application/zip' } else { 'application/vnd.microsoft.portable-executable' }
    $r = Invoke-RestMethod -Method Post -Uri "${uploadBase}?name=$([uri]::EscapeDataString($name))" -Headers $headers `
        -InFile $a -ContentType $type
    Write-Host "  enviado: $($r.name)"
}

Step 'Pronto'
Write-Host $release.html_url
