# Upphone SIP

Softphone SIP para Windows da **Upp Tecnologia**, baseado no [MicroSIP](https://www.microsip.org/) 3.22.16 (edição Lite) e no [pjproject](https://github.com/pjsip/pjproject).

- Tema claro/escuro que acompanha o Windows, com a identidade visual da Upphone
- Interface em português (Brasil), com o langpack embutido no executável
- Codecs G.711 A-law, G.711 u-law e G.729 (bcg729)
- Configurações fixas no código (sem tela de configurações), veja `AccountSettings::SettingsLoad` em `settings.cpp`
- Sem atualização automática, sem envio de relatórios de travamento e sem prazo de validade

## Estrutura

Este repositório é a pasta do aplicativo. Para compilar, ele precisa ficar **dentro** do pjproject:

```
pjproject\                  <- https://github.com/pjsip/pjproject
  pjlib\ pjsip\ pjmedia\ ...
  third_party\
  microsip\                 <- este repositório
    build\build.ps1         <- build completo (32/64 bits + instalador)
    build\Upphone.iss       <- instalador (Inno Setup 6)
    build\pjproject\config_site.h
```

### Versões fixadas

As dependências externas ficam presas a commits exatos em [`build/pins.ps1`](build/pins.ps1). O `build.ps1` baixa e confere essas versões e se recusa a compilar com um pjproject diferente (a não ser com `-AllowOtherPjproject`).

| Dependência | Commit | Observação |
|---|---|---|
| [pjproject](https://github.com/pjsip/pjproject/commit/bd69054a696054db8fb4e62b759a3ecaf26c0189) | `bd69054a69` | master de 09/10/2026 (2.17-dev) |
| [bcg729](https://github.com/BelledonneCommunications/bcg729/commit/fb195811fe630415c9bf433c0c0bebde14314ff2) | `fb195811fe` | G.729, 14/07/2026 |
| [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp/commit/d07876c3090fa57f92af9e7fc92bb4b3f3c27390) | `d07876c309` | inclui o sqlite3, 02/10/2026 |

Para atualizar uma dependência: troque o commit em `pins.ps1`, rode o build completo, teste chamadas e só então publique.

## Pré-requisitos

- Visual Studio Build Tools 2026 com **Desenvolvimento para desktop com C++**, **ATL** e **MFC** do C++ (MSVC mais recente)
- Windows SDK 10.0.26100 ou mais recente
- Git
- Inno Setup 6, apenas para gerar o instalador

## Como compilar

```powershell
git clone https://github.com/Joao-Mondardo-Upp/Upphone_SIP.git
.\Upphone_SIP\build\setup.ps1 -Dir C:\src\upphone
C:\src\upphone\microsip\build\build.ps1 -Installer
```

O `setup.ps1` cria `C:\src\upphone` com o pjproject no commit fixado e coloca este repositório em `C:\src\upphone\microsip`, no mesmo commit do script. Use um caminho curto: o build do pjproject passa do limite de 260 caracteres do Windows em pastas muito longas.

Para compilar uma versão publicada, faça `git checkout v1.0.0` no clone antes de rodar o `setup.ps1`.

O `build.ps1`:

1. aplica `build\pjproject\config_site.h` no pjproject e confere o commit dele;
2. baixa (nas versões fixadas) e compila o **bcg729** (G.729) e o **SQLiteCpp**;
3. compila o pjproject (`Release-Static`) e o `Upphone.exe` para Win32 e x64;
4. copia o resultado para `dist\<versão>\<plataforma>\` e gera os instaladores.

Opções úteis:

| Opção | Efeito |
|---|---|
| `-Platforms x64` | compila só uma plataforma |
| `-SkipPjproject` | não recompila o pjproject (mais rápido) |
| `-AllowOtherPjproject` | aceita um pjproject fora do commit fixado (só para testes) |
| `-PfxPath arquivo.pfx -PfxPassword (Read-Host -AsSecureString)` | assina com um certificado .pfx |
| `-CertThumbprint <hash>` | assina com um certificado instalado ou em token |

## Tema

O app segue a opção do Windows "Escolha o modo de aplicativo padrão" e troca de tema ao vivo. Para forçar um modo (testes ou suporte), defina a variável de ambiente `UPPHONE_THEME=light` ou `UPPHONE_THEME=dark`.

## Licença

GNU GPL v3 (veja `LICENSE`). O MicroSIP e o pjproject são GPL v2 ou posterior e o bcg729 é GPL v3, por isso o conjunto é distribuído sob a GPL v3.

Baseado no MicroSIP, Copyright (C) 2011-2026 MicroSIP (www.microsip.org).
