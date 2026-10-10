# Encaminhamentos da reunião com professor Érico

Data do registro e preparo: 9 de outubro de 2026. Fonte dos requisitos: mensagem do pesquisador e confirmação de **2.500 estações + 1 gateway**. Base: ZIP `relay-halow-tree-main.zip` do Pablo, identificado no manifesto. As anotações recebidas orientam o experimento; documentação e scripts dentro do arquivo não foram tratados como novas ordens do pesquisador.

## Referência inicial

Estrela pura, sem relay, domínio 5 km × 5 km, matriz 50 × 50 com passo de 100 m. Gateway na ponta é a primeira opção preparada; arquivo de posições em `prepared/star-2500-corner/topology.json`. O passo entre pontos é 100 m, mas os 50 pontos ocupam 49 intervalos de 100 m em cada eixo: 4.900 m. Isso cabe no ambiente de 5.000 m sem exigir 51 × 51 estações.

Medir quais estações o gateway consegue atender, preservando todas as 2.500 como população física. Não aguardar associação de todas para começar a janela de tráfego. Fixar antes da execução: prazo de associação, duração de geração, drenagem, carga por direção, MCS, potência, ganho, propagação, canais, beacon, número de slots e duração RAW, fila, seed/run e critério de classificação. As potências assimétricas originais ficam registradas como perfil original, com UL e DL analisados separadamente.

## Definição confirmada de “1 RAW”

O pesquisador confirmou: **um RAW significa um único grupo para todas as estações**. Não alternar páginas como substituto dessa condição. O intervalo convencional do descritor da base é limitado a uma página, mas há uma representação especial de grupo para todas as estações a investigar e implementar no fork. O problema de limite do intervalo não demonstra impossibilidade de um RAW global no padrão.

A implementação deve validar coerência entre autorização RAW, slots no AP e STA, TIM, sono e atendimento DL, incluindo AIDs de páginas diferentes. Um ciclo com dois grupos alternados não equivale ao único grupo solicitado. Detalhes em `RAW_GLOBAL_IMPLEMENTATION.md`; contrato humano em `FOUR_SCENARIO_CONTRACT.json`.

## Etapas posteriores

| Etapa | Topologia / política | Comparação válida |
|---|---|---|
| S1 | Estrela com um único RAW para todas as estações | Cobertura bidirecional, associação e entrega com a geometria original |
| S2 | Mesma estrela com mais de um RAW; 2/3 grupos como subníveis | Mesmos nós, posições, carga, tempo total de acesso e PHY; tratar redistribuição de airtime como fator explícito |
| R1 | Vários relays com um único RAW global | Mesmos relays, pais e rotas usados em R2; coordenação entre BSSs explícita |
| R2 | Mesma rede, cada relay com exatamente um RAW | Política da raiz/backhaul identificada à parte; efeito do calendário local |
| E | Estresse de relays e camadas | Retomar R/D mantendo a referência estrela e o acompanhamento computacional |

Tudo estático durante a medição: posições, papéis de relay, pais/rotas planejados e grupos/calendário. Estados efetivos de associação e entrega são observações, não condições forçadas. Congelar a escolha dos relays após sondagem RSSI/qualidade e reutilizar em R1/R2. Ter um grupo por AP em múltiplos BSSs não demonstra um único grupo RAW coordenado globalmente. Requisitos e controle científico em `FOUR_SCENARIO_METHOD.md`.

## RSSI e qualidade do enlace

RSSI representa potência recebida. Não é uma decisão binária de “traduzível”, e potência alta não garante entrega em presença de ruído/interferência. O fork possui sinal/ruído, mas não um LQI Wi-Fi pronto. Definir o indicador de qualidade antes da seleção de relay, por exemplo entrega/retries/erro por janela com tentativas fracassadas incluídas. A ideia de preferir um sinal mais fraco com boa qualidade deve ser formalizada como uma regra mensurável, com desempenho de ambos os saltos e avanço de cobertura; não atribuir um valor LQI inventado ao modelo.

## Resultado e validade

Exportar cada estação: posição, distância, MAC/AID/página, AP esperado/observado, tempo e estado de associação, recebimento de beacon, autorização RAW, enviados/recebidos únicos/duplicados, entrega UL/DL, atraso e motivo de descarte quando disponível. Separar população total, associada, escalonada e atendida. Fazer curvas por distância e direção, com intervalos por runs. O limiar anterior de perda superior a 5% continua como critério de degradação sob serviço válido.

Antes dos 2.500 nós: regressões de páginas/AID, cópia RPS, associação parcial, início das aplicações e contabilidade; controles pequenos e repetição exata. Falha desses controles é problema de validade a corrigir, não uma medida de alcance.

## Custo computacional e GPU

Registrar tempo de parede, CPU, RSS, volume de logs e motivo de término. Primeiro medir o perfil CPU com modelos e cargas válidos. Avaliar kernels numericamente independentes e replicação de runs separadamente da agenda de eventos. A nota de viabilidade GPU registra hardware observado e fontes primárias; não houve instalação de GPU SDK nem benchmark de aceleração nesta preparação.
