# Método para comparar os quatro cenários estáticos

Registro: 9 de outubro de 2026. Este documento incorpora a definição posterior do pesquisador, também registrada em `MEETING_PROTOCOL_20261009.md` e `FOUR_SCENARIO_CONTRACT.json`. Não executa simulações nem altera a fonte original do Pablo. Quantidade/posição de relays, duração e parâmetros de rádio continuam sendo parâmetros experimentais a registrar, não requisitos atribuídos ao professor.

## Definição dos cenários

| Perfil | Topologia | Política RAW solicitada | População |
|---|---|---|---|
| S1 | Estrela direta para o gateway | Um único grupo global que inclua todas as 2.500 estações | 2.500 estações + 1 AP |
| S2 | Mesma estrela | Mais de um grupo; avaliar 2 e 3 como dois níveis separados | As mesmas 2.500 estações + o mesmo AP |
| R1 | Árvore com relays | Um único grupo RAW global para a rede com relays | Política de papéis/contagem de relays registrada; mesma população de R2 |
| R2 | A mesma árvore de R1 | Cada relay com um único RAW local | A mesma população de R1 |

S2 com dois grupos e S2 com três grupos são duas configurações da segunda maneira de escalar a rede. Uma rodada contendo ambos possui **cinco configurações**, embora compare quatro famílias. Não juntar os resultados de S2(2) e S2(3).

Os quatro perfis não formam um fatorial 2 × 2 puro: “vários grupos na estrela” e “um grupo local por relay” mudam também o escopo e a coordenação dos grupos. A regra de partição de S2 — por AID, posição, RSSI ou qualidade — é outro campo da configuração e fica congelada antes da medição. Se houver comparação entre regras de partição, tratá-las como níveis adicionais; a ordem dos AIDs numa matriz pode criar grupos com distâncias diferentes ao gateway.

Um grupo global significa uma população lógica única e uma política temporal coordenada; reutilizar o identificador “grupo 1” em vários APs independentes não implementa R1. Em R2, registrar ainda a política do RAW do gateway e do backhaul: a exigência de um RAW por relay não define por si só o calendário da raiz, a relação entre BSSs ou a sincronização. Esses campos precisam estar no manifesto antes de executar R1/R2.

Na base examinada, o intervalo convencional de página não representa os 2.500 membros. A codificação especial de grupo para todas as estações é um caminho de implementação ainda sem integração ns-3 validada; ver `RAW_GLOBAL_IMPLEMENTATION.md`. S1 só recebe esse nome após o serviço global ser implementado e validado. Alternar descritores de duas populações em beacons diferentes muda a política e precisa de outro rótulo. Confirmar em trace os AIDs realmente atribuídos; os AIDs planejados não demonstram autorização RAW observada.

## O que permanece estático

Fixar durante cada run: coordenadas, população, papéis, parentes da árvore, rotas, relays selecionados, associação pretendida, grupos RAW e seu calendário periódico. Mudanças periódicas de estado previstas nesse calendário, como dormir e acordar, são parte da política e não alteram a topologia. O estado efetivo de associação pode mudar por falhas e deve ser registrado como resultado.

“Estático” não implica tráfego sincronizado nem ausência de aleatoriedade. Backoff, erro probabilístico do canal e fase das aplicações podem variar entre runs independentes. Para comparar arquiteturas, usar a mesma carga externa e, quando possível, o mesmo trace de geração de pacotes por estação no run correspondente. Não mudar a posição dos nós ou selecionar novos relays após observar o resultado da rodada medida.

## Controle de população, posições e hardware

A referência geométrica confirmada possui AP `(0,0)` e 2.500 estações em `x = 100…5000 m`, `y = 0…4900 m`, espaçadas em 100 m. Preservar esses identificadores e posições como referência. Se os relays forem infraestrutura adicional, informar `R` separadamente: são `2501 + R` nós físicos. Se cada relay possuir dois rádios e os demais nós um, são `2501 + 2R` interfaces de rádio. Essa opção é proposta metodológica, não uma decisão humana já recebida. Contar nós, papéis e transceptores separadamente.

Promover uma estação existente a relay é uma alternativa diferente: requer declarar se ela continua produzindo sua carga de aplicação e altera a composição de papéis. Não fazer essa promoção silenciosamente para manter a contagem total. Ao acrescentar infraestrutura, verificar novamente a separação mínima entre todos os dispositivos e o domínio geométrico.

A malha de 100 m já preenche o interior: um dispositivo novo entre quatro pontos da malha fica a no máximo 70,71 m de uma estação. Manter separação global mínima de 100 m impede inserir relays adicionais nesses espaços internos. As bordas ainda precisam de avaliação explícita; uma política de acrescentar relays pode restringir sua localização e introduzir viés. Designar nós existentes como relays preserva suas posições, mas é outra política de papéis/hardware e precisa manter a carga de aplicação declarada.

Para cada ponto experimental, R1 e R2 devem usar exatamente os mesmos relays, posições, árvore, profundidade, rotas e hardware. A escolha de `R` e da profundidade fica congelada antes do par R1/R2. Quando o objetivo passar a ser estresse por relays/camadas, variar `R` e profundidade como eixos explícitos, sem misturar esse crescimento com o efeito da política RAW.

O perfil de canais é um fator separado:

- **Canal compartilhado:** estações e relays sujeitos ao meio e à interferência representados nesse canal.
- **Perfil de dois rádios/canais da base:** preservar e identificar a separação entre backhaul e acesso; não tratar canais independentes como um único domínio de interferência.

Um relay com dois `WifiNetDevice` no mesmo canal ainda possui dois transceptores no modelo. Isso não demonstra comportamento de um transceptor único. Comparar perfis de canal em blocos separados e rotular as configurações de hardware. Um ganho observado ao adicionar canais mede também recursos adicionais de espectro.

## Matriz e comparações que respondem perguntas diferentes

Para cada combinação previamente registrada de carga, direção, PHY, canais, filas, geometria, quantidade e profundidade de relays, executar S1, S2(2), S2(3), R1 e R2 onde a combinação fizer sentido. Os campos sem relay em S1/S2 ficam como não aplicáveis, não como uma árvore implícita.

| Contraste principal | Questão respondida | Controle indispensável |
|---|---|---|
| S2(2) − S1; S2(3) − S1 | Efeito de dividir a estrela em grupos | Mesmas estações, PHY, carga e orçamento de acesso |
| R1 − S1 | Efeito da infraestrutura relay sob política global | Mesma carga de aplicação; declarar rádios/canais adicionais e número de saltos |
| R2 − R1 | Efeito da política local por relay | Mesma árvore, relays, hardware e carga |
| R2 − S2(k) | Desempenho das alternativas completas | Comparação de arquitetura; não atribuir o resultado apenas ao número de grupos |

O número de transmissões por enlace cresce com o encaminhamento por saltos. Manter a geração fim a fim por estação é o controle da carga externa; reduzir essa geração em R1/R2 para igualar transmissões de rádio mudaria a pergunta. Registrar também a carga oferecida e retransmitida por enlace, como resultado explicativo.

## Orçamento RAW e dois conjuntos de resultados

Fazer primeiro uma comparação com orçamento de acesso controlado. Registrar período do ciclo, slots, duração, população autorizada, oportunidades por estação e uso do canal por beacons, ACKs e dados. Ao passar de um para vários grupos, repartir o orçamento registrado em vez de automaticamente repetir sua duração inteira por grupo.

Em vários BSSs, contabilizar por canal físico: a união dos intervalos autorizados no tempo, as reservas por BSS, as sobreposições e as possibilidades de reúso espacial. A soma de janelas locais sobrepostas não equivale a essa união. R1/R2 precisam explicitar como seus calendários compartilham o canal e o tratamento do backhaul. A sobrecarga adicional de beacons e encaminhamento é um resultado da arquitetura e deve ser medida.

Um segundo conjunto pode usar configurações práticas próprias de cada arquitetura. Nesse conjunto, declarar que o orçamento de acesso, os canais ou o hardware mudaram, apresentar esses valores e medir o benefício junto do custo. Não combinar as duas perguntas em uma curva que atribua todos os ganhos ao RAW.

## Seleção dos relays por RSSI e qualidade

Executar uma fase de sondagem/calibração separada da medição. Registrar candidatos, posições, potência/MCS usados, tamanho e taxa das sondas, prazo de observação e regra de seleção. Definir qualidade operacional por entrega das sondas e custo de retransmissão, incluindo tentativas fracassadas. Avaliar acesso e backhaul, UL e DL; um bom primeiro salto não basta para um caminho fim a fim.

RSSI é uma medida de potência recebida. Não criar um “LQI” como se fosse um atributo Wi-Fi existente sem especificar sua fórmula, escala e validação. A proposta de sinal mais fraco com boa qualidade pode ser formalizada exigindo qualidade mínima e ganho de cobertura, usando RSSI como um componente secundário ou critério de desempate. Não degradar artificialmente um enlace somente para baixar seu RSSI.

Para tentativas que não podem fornecer RSSI decodificado, manter o fracasso no denominador da qualidade e marcar a observação de potência como ausente. Não calcular qualidade somente com os pacotes bem recebidos. Congelar o conjunto selecionado e seus pais para R1/R2. A mesma calibração serve aos dois; selecionar novamente em cada cenário introduziria outro fator.

## Prazo e denominadores de entrega

Fixar prazo de associação, início da geração, janela de geração `T` e prazo final de observação `T + D`, em que `D` é drenagem. Esses valores serão escolhidos após controles/pilotos e congelados na comparação. Começar a geração no prazo estabelecido mesmo que nem todas as estações consigam se associar. Não aguardar associação das 2.500, pois algumas podem permanecer fora de alcance.

Cada mensagem de aplicação gerada tem identificador único, origem, destino e instante de geração. Para cada direção, medir:

- **PDR da população inteira:** mensagens únicas entregues até o prazo final / mensagens geradas para todas as 2.500 estações no período. Em UL, cada estação gera sua carga; em DL, o gateway gera a carga destinada a cada estação. Estações sem associação continuam no denominador. Registrar à parte geração, aceitação pelo socket e envio efetivo à rede.
- **PDR condicional das associadas:** mesma razão, restringindo origem/destino ao conjunto associado ao AP esperado no instante inicial. Congelar esse conjunto durante a contabilidade; uma desconexão posterior não elimina mensagens do denominador. Nos relays, apresentar também o conjunto com caminho completo operacional.
- **Cobertura/serviço:** fração das 2.500 associadas no prazo, fração com caminho válido, autorizadas pelo RAW e atendidas, por direção e distância.

O PDR condicional pode melhorar apenas porque o cenário atende menos estações. Por isso, sempre mostrar PDR total e cobertura ao lado dele. Um subconjunto comum de estações associadas entre os cenários pode ser uma análise secundária, identificado explicitamente.

Mensagens não entregues até o prazo contam como falha do serviço por prazo. Identificar quais ficaram na aplicação, em fila, sem rota, sem associação, expiraram ou foram descartadas. Esse critério não prova que todas as filas estavam vazias nem que toda falha era física. Atraso e percentis são calculados sobre mensagens entregues e devem vir acompanhados da fração perdida.

## Métricas e causas

Por estação, direção e run: geradas, aceitas pela rede, recebidas únicas, duplicadas, perda por prazo, goodput, atraso médio/p50/p95/p99, associação inicial/final, primeiro tempo de associação, desconexões, AP previsto/observado, distância, autorização RAW e saltos.

Por enlace/interface: tentativas MAC, retransmissões, sucesso/fracasso, descartes de fila por motivo, ocupação/espera de fila, TX/RX/CCA/sono, sinal/ruído/SNR quando observáveis, beacons e custo de encaminhamento. Por execução: tempo de parede e CPU, pico de memória residente, eventos/progresso, volume de saída, exit code e motivo de timeout/crash.

**Não somar perdas PHY de todos os receptores e dividir por mensagens geradas.** Um quadro pode ser ouvido e descartado por vários nós, retransmitido, perdido em um salto e recebido depois, ou duplicado na aplicação. Contadores PHY medem eventos de recepção; perda fim a fim mede identificadores únicos não entregues. Quando possível, vincular eventos ao transmissor, receptor pretendido, salto, tentativa e mensagem. Motivos desconhecidos permanecem desconhecidos; não são automaticamente colisões.

O critério anterior de degradação continua sendo **perda estritamente superior a 5%**. Avaliar perda total, condicional e pior estação em painéis separados. Alta perda total por falta de cobertura não é, isoladamente, saturação do escalonamento. Falha de representação RAW ou associação ao AP errado é defeito/invalidade do controle. Crash, timeout, falta de memória e alto custo computacional têm classificação própria.

## Repetições e incerteza

Proposta de trabalho, não uma exigência humana: três runs por configuração para piloto diagnóstico; confirmação com um lote pré-registrado de 30 runs independentes, revisto conforme a precisão necessária e o custo. O piloto escolhe duração e parâmetros; a confirmação usa runs novos e não mistura configurações ajustadas durante o processo.

Reutilizar o número de run correspondente nas configurações comparadas e atribuir streams estáveis à carga de cada estação. Registrar seed, run, RNG, streams, versões e hashes. O manual do ns-3 observa que mudar a configuração pode mudar a atribuição automática de streams; portanto, usar o mesmo seed sozinho não garante o mesmo tráfego externo. Conferir as APIs efetivamente disponíveis neste fork. [Manual ns-3: variáveis aleatórias](https://www.nsnam.org/docs/manual/html/random-variables.html)

A unidade independente é o **run**, não cada pacote ou cada estação de um mesmo run. Calcular uma estimativa por run para cada métrica pré-registrada, apresentar os valores individuais e IC de 95% entre runs. Para média sob condições apropriadas, usar `média ± t(0,975; n−1) × desvio/√n`; examinar distribuição e sensibilidade quando houver massas em 0/100%, poucas repetições ou assimetria. Não tratar 30 como garantia automática de precisão. [NIST: intervalo para a média](https://www.itl.nist.gov/div898/handbook/eda/section3/eda352.htm)

Nos contrastes pareados, calcular primeiro a diferença no par de runs, depois o IC das diferenças; reportar magnitude e incerteza, não somente um teste de significância. [NIST: observações pareadas](https://www.itl.nist.gov/div898/handbook/prc/section3/prc311.htm)

Para uma métrica de perda validada: IC inteiro acima de 5% sustenta degradação nesse ponto; IC inteiro abaixo de 5% sustenta atendimento do critério; IC atravessando o limiar é inconclusivo. Registrar previamente o tratamento de muitos contrastes e de métricas secundárias. Se a curva sugerir um limite, confirmar o intervalo da transição com runs novos, sem publicar como capacidade universal.

## Portões de execução

Antes de publicar comparações: validar cópia/serialização RPS, AIDs/páginas, autorização global de todas as estações, associação parcial, associação ao BSSID esperado, início da aplicação no prazo, contabilidade única e rotas multi-salto. Fazer controles pequenos e depois ampliar preservando o mesmo perfil de modelo. Registrar qualquer alteração corretiva e executar todas as configurações do contraste com a mesma revisão.

Neste preparo, a geometria foi validada, mas não constitui uma simulação dos quatro cenários. A implementação de um grupo global, a política da raiz em R2, seleção/conjunto de relays e calendário entre BSSs precisam de implementação ou configuração explícita antes da campanha.
