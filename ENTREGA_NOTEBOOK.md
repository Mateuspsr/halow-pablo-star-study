# Entrega para demonstração no notebook

Este pacote contém a base do Pablo, os arquivos de geometria, a documentação dos quatro cenários, os componentes isolados de RAW/LQI/RSSI e os testes reproduzíveis.

## O que será mostrado

1. A topologia planejada de 2.500 estações mais um gateway em uma área de 5 km × 5 km.
2. A verificação das 3.126.250 duplas de posições, com distância mínima de 100 m.
3. O teste da regra de pertencimento de um RAW global para AIDs de páginas diferentes.
4. O teste do indicador de qualidade experimental e da contabilidade de RSSI, retries e perdas.
5. A documentação do que ainda precisa ser integrado ao ns-3 antes de apresentar resultados de cobertura ou PDR.

## Execução rápida no Windows

Abra o PowerShell na pasta deste arquivo e execute:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\tools\run_notebook_demo.ps1
```

O script compila e executa os testes isolados, mostra os logs, abre o mapa `prepared\star-2500-corner\layout.svg` e abre o `study\STATUS.md` no Bloco de Notas. O mapa é uma geometria planejada; as cores indicam páginas de AID previstas, não alcance de rádio.

## Execução no Ubuntu/WSL

```bash
bash tests/run-component-checks.sh
xdg-open prepared/star-2500-corner/layout.svg
```

Se `xdg-open` não estiver disponível, abra o arquivo SVG pelo Explorador de Arquivos do Windows.

## Como explicar a demonstração

Os testes isolados não executam o ns-3 e não fabricam resultados físicos. Eles validam regras que precisam estar corretas antes da simulação: associação ao RAW, identificação de probes, deadline, duplicatas, retries e fórmula de potência recebida.

O grupo RAW global usa uma representação especial para todas as estações. Ela ainda precisa ser integrada ao AP, às STAs, ao TIM e ao sono do fork. O código original permanece intacto; os componentes novos ficam fora de `source`.

## O que ainda não deve ser apresentado como resultado

Não há ainda execução ns-3 com 2.500 estações, cobertura, PDR, comparação dos quatro cenários, limite de relays, limite de camadas ou speedup de GPU. Esses resultados só são válidos depois dos gates descritos em `study/STATUS.md`, `study/RAW_GLOBAL_IMPLEMENTATION.md` e `study/FOUR_SCENARIO_METHOD.md`.

## Arquivos principais

- `source/relay-halow-tree-main`: fonte recebida do Pablo, preservada por hash.
- `prepared/star-2500-corner/layout.svg`: mapa visual.
- `prepared/star-2500-corner/topology.json`: posições e contagem.
- `implementation/raw-membership.h`: regra isolada de RAW global.
- `implementation/link-quality.h/.cc`: janela de qualidade por enlace.
- `tests/run-component-checks.sh`: testes reproduzíveis.
- `study/FOUR_SCENARIO_CONTRACT.json`: contrato dos quatro ambientes.
- `study/FOUR_SCENARIO_METHOD.md`: método científico.
- `study/LQI_RSSI_VALIDATION.md`: validação proposta de RSSI/LQI.
- `study/RAW_GLOBAL_IMPLEMENTATION.md`: integração necessária do RAW.
- `study/STATUS.md`: estado e demandas restantes.
