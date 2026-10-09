# Upphone SIP

Softphone SIP para Windows da Upp Tecnologia, baseado no MicroSIP 3.22.16 (Lite, MFC/C++) e no pjproject. Antes de mudar algo, leia o histórico do projeto e as decisões tomadas em [docs/HISTORICO.md](docs/HISTORICO.md).

- Converse com o usuário em português do Brasil.
- Este repositório fica dentro do pjproject (`<pjproject>\microsip`). Monte o ambiente com `build\setup.ps1` e compile com `build\build.ps1` (veja o README).
- As versões das dependências estão fixas em `build\pins.ps1`. Não atualize sem pedido explícito e sem um teste completo.
- As configurações do app são fixas no código (`AccountSettings::SettingsLoad` em `settings.cpp`) e a tela de configurações está desativada de propósito.
- Cores: use as macros `_GLOBAL_THEME_*` e `_GLOBAL_BRAND_ACCENT` (`define.h`, `Theme.h`), nunca RGB fixo, para funcionar nos temas claro e escuro.
- Marque as alterações em relação ao MicroSIP com comentários `// Upphone: ...`.
- Nunca faça commit de `*.ini`, `*.db` ou `Contacts.xml`: o `Upphone.ini` contém a senha do ramal.
- Para testar a interface, capture só a janela do próprio app (`PrintWindow`). Nunca capture a tela inteira nem mande mensagens para a janela em primeiro plano.
