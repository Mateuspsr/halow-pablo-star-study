# Estado em 9 de outubro de 2026

Base confirmada: ZIP `relay-halow-tree-main.zip` do Pablo, SHA-256 `9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6`. Foram materializados 3.324 arquivos; 35 entradas geradas, links simbólicos ou nomes não portáveis ficaram registradas como não materializadas e preservadas no arquivo recebido. A verificação de todos os arquivos importados confirmou **zero alterações na fonte**, em `provenance/source-verification.json`. A primeira importação, interrompida em um link simbólico de documentação, foi preservada em `provenance/partial-import-source-20261009`; a importação atual completou a validação prévia de todos os caminhos.

## Concluído nesta preparação

- Identificado `scratch/test` como cenário estrela de referência e `scratch/test++` como relays da versão do Pablo.
- Confirmada a contagem humana: 2.500 estações e um AP, total 2.501.
- Preparada estrela geométrica com AP em `(0,0)`, matriz 50×50 com `x=100..5000`, `y=0..4900` e passo 100 m, dentro de 5 km × 5 km.
- Validadas independentemente todas as 3.126.250 duplas: mínima global 100 m, máxima distância ao AP 7.000,714249 m, sem sobreposição. Arquivos em `prepared/star-2500-corner/`.
- Auditado o formato RAW, início de aplicações, população, potência, AID/página, mapas esparsos e cópia RPS. Achados em `PABLO_BASE_AUDIT.md`.
- Avaliada possibilidade de GPU com documentação primária e hardware local. O fork não oferece offload automático; CPU/replicações e kernels numéricos específicos são caminhos distintos, ainda sem benchmark. Nota em `GPU_FEASIBILITY.md`.
- Registradas as quatro comparações posteriores e o cuidado com LQI/RSSI no protocolo da reunião.

## Definição humana resolvida

O pesquisador confirmou quatro cenários estáticos: S1 estrela/um RAW para todas as estações; S2 estrela/mais de um RAW; R1 vários relays/um RAW global; R2 vários relays/cada relay com exatamente um RAW. A alternância de páginas não satisfaz o RAW único solicitado. Contrato em `FOUR_SCENARIO_CONTRACT.json`, método em `FOUR_SCENARIO_METHOD.md`.

A geometria, a contagem e o significado de um RAW estão resolvidos por confirmação humana. Permanece trabalho de implementação, não pergunta pendente sobre esse significado.

## Componentes novos e verificações isoladas

- Investigada a exceção RAW Group zero para todas as estações; identificada em fonte primária histórica IEEE e no dissector atual do Wireshark. A restrição de onze bits vale para intervalo de página; não demonstra impossibilidade de grupo global. O fork não implementa a exceção. Detalhes em `RAW_GLOBAL_IMPLEMENTATION.md`.
- Criado `implementation/raw-membership.h`: pertencimento de AIDs associados de todas as páginas, intervalo convencional e slot pelo AID completo. Teste standalone: 32.788 verificações, zero falhas, população planejada de 2.500 coberta pela regra. Isso não é observação de autorização em uma simulação.
- Criados `implementation/link-quality.h/.cc`: coorte de sondas de um salto por enlace/interface/direção; entrega única, duplicações, prazo de drenagem, tentativas/retries separados, RSSI e SNR trace identificado. Proposta de LQI experimental = 100 × PDR de sondas, sem atribuir indicador nativo ao Wi-Fi.
- Testes standalone de qualidade: 64 checks aprovados com dados sintéticos; incluem contabilidade, janelas, direções/interfaces, falta de dados, perdas completas, RF e fórmula analítica exata do fork. Vinte sondas é apenas um mínimo configurável proposto, sem garantia de precisão estatística.
- Verificação reproduzível: `tests/run-component-checks.sh`; logs e hashes em `validation/`. A fonte original não recebe esses módulos automaticamente.

## Implementação e validação de rede ainda necessárias

Integrar grupo global com AP/STA/TIM/sono, acordar para DL mesmo com fila UL vazia, corrigir propriedade RPS/TIM e acessos a mapas esparsos. Adaptar a estrela para a matriz, população física independente do RAW e janela fixa de tráfego. Aplicar o mesmo monitor aos quatro cenários, validar RSSI por enlace/direção e integrar sondagem de qualidade sem excluir fracassos do denominador.

Para R1, implementar um único domínio de calendário/participantes entre BSSs; copiar wildcard para cada relay não basta. Para R2, explicitar calendário do gateway/backhaul. Registrar contagem/posição/camadas dos relays, se adicionais ou papéis de estações existentes, canais/rádios e orçamento RAW; preservar a mesma árvore e carga em R1/R2.

Depois dos controles pequenos UL, DL e bidirecional: executar os quatro cenários na escala preparada e comparar cobertura, entrega, atraso, filas/retries e custo computacional. A adaptação do cenário ns-3 e a simulação de 2.500 estações ainda não foram executadas. Não existem resultados de cobertura, PDR de pesquisa ou speedup nesta etapa.
