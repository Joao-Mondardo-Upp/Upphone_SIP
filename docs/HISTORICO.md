# Upphone SIP: histórico e passagem do projeto

Documento para continuar o trabalho em outra máquina. Resume o que foi feito no desenvolvimento da versão 1.0.0 (outubro de 2026), por que cada decisão foi tomada e o que está pendente.

- Repositório: https://github.com/Joao-Mondardo-Upp/Upphone_SIP (público)
- Release publicada: [v1.0.0](https://github.com/Joao-Mondardo-Upp/Upphone_SIP/releases/tag/v1.0.0)
- Como montar o ambiente e compilar: [README.md](../README.md)

---

## 1. Estado atual

**Upphone SIP 1.0.0 está publicado**, com instaladores e versões portáteis de 32 e 64 bits. Os binários da release foram compilados a partir da tag `v1.0.0` (commit `18dbd68`) numa pasta limpa, com `build/setup.ps1` e `build/build.ps1 -Installer`.

| Item | Situação |
|---|---|
| Identidade visual Upphone (nome, ícones, cores) | Pronto |
| Tema claro/escuro seguindo o Windows, com troca ao vivo | Pronto e testado |
| Português (Brasil) | Pronto |
| Codecs G.711 A-law, u-law e G.729 | Compilado. **Falta testar G.729 numa chamada real** |
| Build reproduzível (versões fixadas) | Pronto e testado do zero |
| Instalador (Inno Setup) | Pronto e testado em outro computador |
| Release no GitHub | Pronto (`build/release.ps1`) |
| Assinatura digital | **Pendente**, depende do certificado da Upp |
| Teste de chamada real (áudio nos dois sentidos) | **Pendente** |

---

## 2. Origem do código

- Base: **MicroSIP 3.22.16, edição Lite**, código-fonte de https://www.microsip.org/source. O pacote recebido já vinha personalizado: **configurações fixas no código e tela de configurações desativada** (`AccountSettings::SettingsLoad` em `settings.cpp` e `CmainDlg::OnMenuSettings` vazio em `mainDlg.cpp`). Isso foi mantido de propósito.
- O MicroSIP não traz as bibliotecas de terceiros. Ele precisa ficar **dentro do pjproject**, como `<pjproject>\microsip`, e é compilado junto com ele.
- Dependências, fixadas em `build/pins.ps1`:
  - pjproject `bd69054a696054db8fb4e62b759a3ecaf26c0189` (master de 09/10/2026, 2.17-dev). Foi identificado pelo comentário do ZIP baixado do GitHub e conferido arquivo por arquivo contra a cópia usada no build.
  - bcg729 `fb195811fe630415c9bf433c0c0bebde14314ff2` (G.729)
  - SQLiteCpp `d07876c3090fa57f92af9e7fc92bb4b3f3c27390` (inclui o sqlite3)

---

## 3. O que foi feito, em ordem

1. **Cores e logo (primeira versão, depois substituída).** Paleta tirada de um print (azul-marinho e âmbar) e logo no topo do discador.
2. **Ícone do app** trocado pela logo laranja da Upphone (`res/microsip.ico`).
3. **Ambiente de build.**
   - Instalação do pjproject e criação do `config_site.h`.
   - Inclusão do MFC no Visual Studio Build Tools 2026.
   - Build do SQLiteCpp à mão com `cl`, porque não havia CMake.
   - Correção de duas incompatibilidades com as versões novas, ambas no `microsip.vcxproj`:
     - `SQLITECPP_DISABLE_STD_FILESYSTEM`, porque o MicroSIP não compila em C++17;
     - `PJMEDIA_AUD_MAX_DEVS=PJMEDIA_AUD_DEV_MAX_DEVS`, nome que mudou no pjproject.
4. **Vídeo, DTLS, Opus e G.729 desligados** no primeiro build (`_GLOBAL_VIDEO` em `const.h`, `_GLOBAL_DTLS` em `define.h`). O G.729 voltou depois (item 16).
5. **Tema escuro**, a pedido de deixar o app "todo preto com destaques nos botões". O destaque é o laranja da logo, `RGB(228,103,55)`. A janela principal do MicroSIP não tinha tema escuro, então foi preciso desenhar vários controles:
   - abas: `DarkTabCtrl` (classe nova);
   - botão de menu ▼: `CDarkArrowButton`;
   - teclado: `ButtonDialer`;
   - botões DND/AA: `ButtonBottom`;
   - botão Chamar: `ButtonEx`;
   - barra de status: `StatusBar`;
   - sliders de volume: `LevelsSliderCtrl`;
   - listas, cabeçalhos e campos de texto: `ThemeDialog`.
6. **Português (Brasil).** O langpack oficial `langpack_portuguesebr.txt` do microsip.org (100% traduzido) foi **embutido no .exe** (RCDATA `IDR_LANGPACK_PTBR`). Se não houver um `langpack_*.txt` ao lado do .exe, `lib/langpack.cpp` extrai o embutido para a pasta temporária e carrega.
7. **Logo grande removida** do topo do discador (`_GLOBAL_LOGO_HEIGHT 0`).
8. **Espaços vazios eliminados.**
   - A faixa inferior das abas foi encolhida.
   - A altura da janela principal agora é sempre a padrão. A altura salva no `.ini` é ignorada (`mainDlg.cpp`, `int mH = rect.Height()`).
   - As páginas Registros e Contatos ocupam exatamente o espaço entre as abas e a barra de status.
9. **Aviso de atualização do MicroSIP desativado** (`CheckUpdates` retorna direto), porque oferecia trocar o app pelo MicroSIP original.
10. **Linha azul dos campos de texto** (foco do Windows 11) trocada por uma borda própria: cinza, e laranja quando o campo está em foco.
11. **Ícones da bandeja** refeitos com a logo da Upphone e um ponto de status. Os originais do MicroSIP ficaram fora do repositório, em `microsip-icons-original\`, na máquina original.
12. **Nome visível "Upphone SIP"** (`_GLOBAL_NAME_NICE`). O nome interno `_GLOBAL_NAME` continua "MicroSIP", porque é usado na classe da janela e para detectar a instância já aberta.
13. **Botões CONF e REC removidos** (`Dialer.cpp`, `addConf`/`addRec = false`).
14. **Links do menu** (site, ajuda, downloads, reportar problema) apontam para https://upphone.com.br/.
15. **Registros e Contatos**:
    - cabeçalho das colunas legível;
    - rodapé alinhado;
    - lupa da busca em 16 px (`CBaseDialog::FitSearchIcon`).
16. **G.729 de volta**, porque o PABX da Upp exige G.711 A-law, G.711 u-law e G.729.
    - Biblioteca **bcg729**, estática (`BCG729_STATIC`), com `PJMEDIA_HAS_BCG729 1` no `config_site.h`.
    - A lista fixa de codecs é `PCMA/8000/1 PCMU/8000/1 G729/8000/1` (`settings.cpp`).
17. **Relatório de travamento desligado.** O app não envia mais nada para `crash-report2.microsip.org`; o arquivo `crash-dump_*.txt` continua sendo salvo localmente.
18. **`Upphone.exe`** (`TargetName` no `vcxproj`).
    - O `Upphone.ini` é criado a partir do `microsip.ini` automaticamente (`settings.cpp`).
    - A entrada antiga "MicroSIP" da inicialização automática é removida (`global.cpp`, `msip_startup_set`).
19. **Versão 1.0.0.** `_GLOBAL_VERSION`, `_GLOBAL_VERSION_COMMA` e `_GLOBAL_TIMESTAMP` em `const.h`. Copyright e empresa "Upp Tecnologia" nas propriedades do `.exe`, mantendo o crédito ao MicroSIP.
20. **Prazo de validade desativado.** O MicroSIP bloqueava o app 2,5 anos depois do build; agora ele nunca expira (`microsip.cpp`, `false &&` nas checagens de `lifeTime`).
21. **Tema claro/escuro seguindo o Windows** (`Theme.h/.cpp`).
    - A paleta é dinâmica; o laranja de destaque é igual nos dois modos.
    - A troca ao vivo é feita por `WM_SETTINGCHANGE "ImmersiveColorSet"`, em `CThemeDialog::OnSettingChange`, que também redesenha as páginas filhas.
    - Para forçar um modo: variável `UPPHONE_THEME=light|dark`.
22. **Script de build** `build/build.ps1`.
    - Compila Win32 e x64, detectando o toolset e o SDK.
    - Assinatura opcional com `-PfxPath` ou `-CertThumbprint`.
    - Gera o instalador Inno Setup com `-Installer`.
23. **Instalador** `build/Upphone.iss`.
    - Em português; instala em Arquivos de Programas.
    - Grava `HKLM\Software\Upphone SIP` com a pasta da instalação. Com isso o app entra em modo instalado e guarda as configurações em `%APPDATA%\Upphone SIP\`.
24. **Git e GitHub.**
    - `.gitignore` exclui `.ini`, `.db`, builds e `dist`.
    - README e LICENSE (GPL v3).
25. **Versões fixadas.** `build/pins.ps1` e `build/setup.ps1`, testados com um build do zero a partir do GitHub.
26. **Release v1.0.0 publicada** e script `build/release.ps1` para as próximas.
27. **Telas de chamada e listas no tema** (depois da 1.0.0). Revisadas nos temas claro e escuro, em 125% de escala.
    - Chamada recebida: Atender em laranja e Recusar em vermelho, como Chamar e Terminar chamada (`RinginDlg`).
    - Janela de mensagens: as abas (`CClosableTabCtrl`) são desenhadas no tema, com o "x" de fechar e as setas de rolagem próprias; o histórico e o campo de texto têm a borda e a barra de rolagem do tema.
    - Registros e Contatos: as colunas ocupam a largura da lista, sem barra horizontal. Quando a janela é estreita, as últimas colunas (Duração, Informação) ficam ocultas e voltam ao alargar; arrastar uma divisória muda a proporção (`CListCtrl_Sortable::FitColumnsEnable`).
    - Seta de ordenação desenhada à direita do nome da coluna (`ThemeSetListSort`), porque a do tema do Windows fica no topo do cabeçalho, colada nas abas.

---

## 4. Decisões e motivos

| Decisão | Motivo |
|---|---|
| Sem tela de configurações | Já vinha assim no código recebido: os valores são fixos para os clientes |
| G.711 A-law, u-law e G.729 | Exigência do PABX da Upp |
| Sem vídeo, Opus e TLS/SRTP | Não eram necessários na 1.0.0 e reduzem dependências (OpenSSL, ffmpeg etc.) |
| Sem atualização do MicroSIP | Ela substituiria o Upphone pelo MicroSIP original |
| Sem prazo de validade | Pedido da Upp ("prazo infinito"); o app não se atualiza sozinho |
| Sem relatório de travamento | Os dados iriam para os desenvolvedores do MicroSIP |
| Sem CONF e REC | Pedido da Upp |
| Nome interno "MicroSIP" mantido | Classe da janela e detecção de instância única; trocar não traz benefício visível |
| Licença GPL v3 | MicroSIP e pjproject são GPL v2 ou posterior; o bcg729 é GPL v3 |
| Repositório público | Escolha da Upp. A GPL exige entregar o código-fonte a quem recebe o app |

---

## 5. Mapa do código (o que foi criado ou alterado)

| Arquivo | O que tem |
|---|---|
| `Theme.h/.cpp` | Paleta claro/escuro, leitura do modo do Windows, `UPPHONE_THEME`, coluna ordenada das listas |
| `define.h` | `_GLOBAL_BRAND_ACCENT`, macros `_GLOBAL_THEME_*`, links upphone.com.br, nome "Upphone SIP" |
| `const.h` | Versão 1.0.0, timestamp, empresa, vídeo desligado |
| `ThemeDialog.h/.cpp` | Cores dos controles (`OnCtlColor`), borda dos campos e do chat, cabeçalho e seta de ordenação das listas, troca de tema ao vivo |
| `ClosableTabCtrl.h/.cpp` | Abas da janela de mensagens desenhadas no tema, botão de fechar e setas de rolagem |
| `CListCtrl_Sortable.h/.cpp` | Colunas ajustadas à largura da lista, seta de ordenação via `ThemeSetListSort` |
| `RinginDlg.h/.cpp` | Cores dos botões Atender e Recusar |
| `DarkTabCtrl.h/.cpp` | Abas desenhadas (`CDarkTabCtrl`) e botão de menu ▼ (`CDarkArrowButton`) |
| `ButtonDialer.cpp` | Teclas do discador |
| `ButtonBottom.*`, `ButtonEx.*` | Botões DND/AA e Chamar |
| `StatusBar.*` | Barra de status desenhada |
| `LevelsSliderCtrl.cpp` | Sliders de volume |
| `BaseDialog.*` | `FitSearchIcon` (lupa da busca) |
| `Calls.cpp`, `Contacts.cpp` | Lupa, fundo dos ícones, larguras mínimas das colunas |
| `Dialer.cpp` | Sem CONF/REC, cores do botão Chamar, sliders |
| `MessagesDlg.cpp` | Cores do chat, espaço para o botão de fechar nas abas |
| `mainDlg.h/.cpp` | Abas, menu, altura fixa, páginas sem espaço vazio, sem atualização do MicroSIP, biblioteca do pjproject por arquitetura, `bcg729.lib` |
| `microsip.cpp` | Menus no tema do sistema, sem relatório de travamento, sem prazo de validade |
| `settings.cpp` | Codecs, migração `microsip.ini` → `Upphone.ini` |
| `global.cpp` | Remove a entrada antiga "MicroSIP" da inicialização |
| `lib/langpack.cpp` | Langpack embutido |
| `res/` | Ícones (app e bandeja), `langpack_portuguesebr.txt`, `main.rc2` (recursos e versão), `dialog.rc2` (layout) |
| `microsip.vcxproj` | `Upphone.exe`, defines de compatibilidade, arquivos novos |
| `build/` | `build.ps1`, `setup.ps1`, `pins.ps1`, `release.ps1`, `Upphone.iss`, `pjproject/config_site.h`, `notas/` |

Várias alterações em arquivos do MicroSIP estão marcadas com `// Upphone:` (`git grep "Upphone:"`), mas nem todas. O primeiro commit deste repositório (`0c654b0`) já contém as alterações da 1.0.0. Para ver tudo o que mudou em relação ao MicroSIP original, compare com o código-fonte 3.22.16 de https://www.microsip.org/source.

---

## 6. Armadilhas conhecidas

- **Caminho curto.** Monte o build em algo como `C:\src\upphone`. Caminhos longos estouram o limite de 260 caracteres no build do pjproject.
- **PowerShell bloqueia scripts** num Windows novo: rode com `powershell -ExecutionPolicy Bypass -File ...`.
- **MFC e ATL** precisam estar marcados no Visual Studio Installer (Build Tools → Modificar → "ATL do C++" e "MFC do C++"). Sem eles, o erro é `MSB8041`.
- **Toolset:** o pjproject pede v143, mas o `build.ps1` passa `BuildToolset` e `PlatformVersion` detectados automaticamente (hoje v145 e SDK 10.0.26100).
- **PowerShell 5.1:**
  - texto enviado por pipe a programas nativos chega alterado, por isso o `release.ps1` usa um arquivo temporário para ler a credencial;
  - `ConvertTo-Json` com `Get-Content -Raw` gera JSON inválido;
  - `"$var?x"` é lido como nome de variável (use `"${var}?x"`).
- **Controle Inteligente de Aplicativos (Windows 11)** bloqueia o instalador **sem a opção "Executar assim mesmo"**, porque ele não tem assinatura digital. Só a assinatura resolve. Para testar enquanto isso: use um PC sem esse recurso ou desligue-o num PC de teste (pode ser difícil religar).
- **SmartScreen:** "Mais informações → Executar assim mesmo".
- **Duas cópias abertas** (portátil e instalada) entram em conflito, porque o app aceita uma só instância.
- **Inicialização automática:** o app sempre se registra em `HKCU\...\Run` com o caminho do `.exe` que está rodando. Ao testar outro build, o registro passa a apontar para ele.
- **Testar telas de chamada sem PABX:** o app pode ligar para ele mesmo (`sip:teste@127.0.0.1`) para abrir a chamada recebida. Para uma chamada em andamento estável, use um atendedor SIP local que responda 200 OK. Envie os comandos (número, `/answer`, `/hangupall`) por `WM_COPYDATA` à janela de classe `MicroSIP` do processo `Upphone`: pela linha de comando o app chama `SetForegroundWindow`, e o `Start-Process` do PowerShell 5.1 altera os argumentos (`/minimized` vira um "Número inválido").

---

## 7. Pendências e próximos passos

Em ordem de prioridade:

1. **Teste de chamada real** no PABX: áudio nos dois sentidos e G.729 forçado no ramal.
2. **Assinatura digital.** Recomendado: Microsoft Trusted Signing (Azure Artifact Signing), cerca de US$ 10 por mês e reconhecido pelo Controle Inteligente de Aplicativos. Alternativa: certificado OV/EV de uma autoridade (DigiCert, Sectigo). O `build.ps1` já assina com `-PfxPath` ou `-CertThumbprint`; com o Trusted Signing será preciso adaptar.
3. **Escala de tela** de 150%: 100% e 125% já foram testados.
4. **Detalhes visuais restantes** (vistos na revisão do item 27 da seção 3):
   - botões Espera e Mudo ligados ficam em cinza do Windows, sem o laranja de destaque;
   - teclado do diálogo Transferir com botões padrão, diferentes das teclas do discador;
   - "Call on Local Hold" sem tradução no langpack;
   - na barra de status, a Espera aparece sem ícone.
5. **Integrações sugeridas:**
   - abrir a ficha do cliente quando o telefone toca: o app já tem os ganchos `cmdIncomingCall`, `cmdCallAnswer` etc., que podem chamar um webhook do n8n e, a partir dele, um CRM ou helpdesk;
   - click-to-call (`tel:`/`sip:`);
   - provisionamento automático da conta;
   - atualização automática própria, apontando para upphone.com.br.
6. **Opcionais:** TLS/SRTP (exige OpenSSL) e Opus.

---

## 8. Como retomar em outra máquina

1. Instale o Git, o Visual Studio Build Tools 2026 (com **C++**, **ATL** e **MFC**) e, para gerar o instalador, o Inno Setup 6. Detalhes no README.
2. Monte o ambiente:
   ```powershell
   git clone https://github.com/Joao-Mondardo-Upp/Upphone_SIP.git
   powershell -ExecutionPolicy Bypass -File .\Upphone_SIP\build\setup.ps1 -Dir C:\src\upphone
   powershell -ExecutionPolicy Bypass -File C:\src\upphone\microsip\build\build.ps1 -Installer
   ```
3. Trabalhe em `C:\src\upphone\microsip`, sempre com `git pull` ao começar e `git push` ao terminar.
4. Cadastre o ramal de novo no app: o `Upphone.ini` não vai para o git, porque tem a senha.
5. Para publicar uma versão nova, siga a seção "Publicar uma versão" do README (`build/release.ps1`).
6. Se usar o Claude Code, abra a pasta `C:\src\upphone\microsip`. Ele lê o `CLAUDE.md` e este histórico automaticamente.
