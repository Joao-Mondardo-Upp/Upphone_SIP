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

Versão de pjproject usada no 1.0.0: `master` de 09/10/2026 (2.17-dev).

## Pré-requisitos

- Visual Studio Build Tools 2026 com **Desenvolvimento para desktop com C++**, **ATL** e **MFC** do C++ (MSVC mais recente)
- Windows SDK 10.0.26100 ou mais recente
- Git
- Inno Setup 6, apenas para gerar o instalador

## Como compilar

```powershell
git clone https://github.com/pjsip/pjproject.git
cd pjproject
git clone https://github.com/Joao-Mondardo-Upp/Upphone_SIP.git microsip
cd microsip\build
.\build.ps1 -Installer
```

O script:

1. aplica `build\pjproject\config_site.h` no pjproject;
2. baixa e compila o **bcg729** (G.729) e o **SQLiteCpp**;
3. compila o pjproject (`Release-Static`) e o `Upphone.exe` para Win32 e x64;
4. copia o resultado para `dist\<versão>\<plataforma>\` e gera os instaladores.

Opções úteis:

| Opção | Efeito |
|---|---|
| `-Platforms x64` | compila só uma plataforma |
| `-SkipPjproject` | não recompila o pjproject (mais rápido) |
| `-PfxPath arquivo.pfx -PfxPassword (Read-Host -AsSecureString)` | assina com um certificado .pfx |
| `-CertThumbprint <hash>` | assina com um certificado instalado ou em token |

## Tema

O app segue a opção do Windows "Escolha o modo de aplicativo padrão" e troca de tema ao vivo. Para forçar um modo (testes ou suporte), defina a variável de ambiente `UPPHONE_THEME=light` ou `UPPHONE_THEME=dark`.

## Licença

GNU GPL v3 (veja `LICENSE`). O MicroSIP e o pjproject são GPL v2 ou posterior e o bcg729 é GPL v3, por isso o conjunto é distribuído sob a GPL v3.

Baseado no MicroSIP, Copyright (C) 2011-2026 MicroSIP (www.microsip.org).
