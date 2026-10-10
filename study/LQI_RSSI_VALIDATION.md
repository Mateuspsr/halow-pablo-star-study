# Qualidade por enlace e validação RSSI

Data: 9 de outubro de 2026. Base: ZIP do Pablo, SHA-256 `9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6`. Esta entrega é revisão de código, definição operacional e testes unitários de um módulo separado. Nenhuma simulação HaLow de alcance/seleção foi executada por este trabalho. Nenhum arquivo em `source` foi alterado.

## O que existe e o que precisa ser validado

O monitor RSSI existe em `source/relay-halow-tree-main/scratch/test++/monitor.h:128–184`. É instalado pela arquitetura com relays em `scratch/test++/s1g-test-tim-raw.cc:1473` e exportado em `:1585`. A estrela de `scratch/test` não inclui nem instala esse monitor; possuir o arquivo no repositório não significa que a estrela exporte RSSI.

`MonitorSnifferRx` fornece sinal e ruído após recepção bem-sucedida da PHY: `src/wifi/model/yans-wifi-phy.cc:1249–1265`. Portanto, a média observada não inclui os quadros que a PHY perdeu. Uma estação sem amostras não tem RSSI zero; tem valor ausente. O receptor pode ouvir quadros destinados a outros nós. Para relacionar RSSI aos probes, filtrar classe, emissor, receptor pretendido e interface, separando beacon, dados, ACK e outras categorias.

O resumo `g_rssiNodeAp`, em `monitor.h:173–177`, usa apenas o nó como chave e agrega AP ouvindo STA com STA ouvindo AP. Também inclui nós ouvindo outros APs/relays. Assim, mistura direções, interfaces e possíveis parceiros; não comprova qualidade do enlace específico até o gateway. A potência AP/STA é assimétrica, o que torna essa mistura especialmente inadequada. A busca `g_nodeRole[sender]` ainda insere um papel default para transmissor desconhecido; evitar essa classificação implícita na instrumentação nova.

O vetor `g_rssiHistory` cresce por cada recepção mesmo com `g_logPacketRecords=false`; essa opção reduz registros IP, não o histórico RF. Para 2.500 estações, agregar por janela/interface sem guardar todos os quadros é uma opção a medir. O módulo abaixo usa médias online, mas conserva identidades de probes/tentativas para deduplicação: seu custo ainda cresce com a quantidade de probes e tentativas na janela.

Não foi encontrado LQI pronto no módulo Wi-Fi desta base. O `LrWpanLqiTag` do repositório pertence a IEEE 802.15.4; transportar sua escala para HaLow seria outro modelo, sem equivalência validada. A [documentação primária LR-WPAN do ns-3](https://www.nsnam.org/docs/models/html/lr-wpan.html) identifica LQI como informação da associação IEEE 802.15.4. Neste estudo, usamos um indicador experimental definido abaixo, com nome e unidade explícitos.

## Definição operacional proposta

Para cada enlace **dirigido** `txNode, txInterface → rxNode, rxInterface`, gerar probes de um salto em `[inicio, fim)` e admitir recebimentos até, mas sem incluir, `deadline=fim+drenagem`. Essa convenção exclui o instante exato do deadline para não depender da ordem dos callbacks empatados. Identidade de probe é única por enlace durante toda a janela.

```text
PDR_probe = probes únicos recebidos antes do deadline / probes únicos gerados
LQI_exp_percent = 100 × PDR_probe
não_entregues_no_prazo = gerados únicos − recebidos únicos
retries = soma, por probe, de max(tentativas físicas únicas − 1, 0)
```

`LQI_exp_percent` mede entrega do serviço de probes sob os parâmetros registrados. É uma transformação explícita de PDR, não uma informação nativa do rádio. Inclui efeitos de fila, agendamento RAW, acesso ao canal, retries e PHY. Não mede isoladamente erro físico, nem substitui sucesso confirmado por ACK. Pacotes gerados que não chegaram à PHY continuam no denominador; registrar motivos de fila/RAW separadamente. Falha de envio local precisa ser contada/classificada pelo adaptador.

Antes do deadline, os não recebidos ficam pendentes. Após encerramento, tornam-se não entregues no prazo, inclusive se continuarem em fila ou forem entregues depois. Isso não prova fila vazia. O limiar de perda superior a 5% aplica-se a essa definição somente com associação/RAW/instrumentação válidos; não confundir exclusão de página ou ausência de associação com perda física.

Nenhum probe enviado significa PDR/LQI indefinido. A proposta implementada requer pelo menos **20 probes** para expor `LQI_exp_percent`; esse mínimo é configurável e constitui um controle de suficiência operacional, não significância estatística. Para pesquisa, fixar número/cadência, repetir runs independentes e reportar incerteza por run. Vinte probes sem perdas não demonstram que a taxa real de perdas seja menor que 5%. O módulo não calcula intervalos de confiança nem cria réplicas artificiais de quadros.

## RSSI, ruído do trace e SNR efetivo

Conservar os campos originais `signal_dbm` e `noise_trace_dbm`. O código do fork usa:

```text
signal_dbm = 10 log10(P_rx_W) + 30
noise_trace_dbm = 10 log10(P_rx_W / snrPer.snr) − RxNoiseFigure_dB + 30
snr_trace_difference_db = signal_dbm − noise_trace_dbm
effective_snr_scalar_db = snr_trace_difference_db − RxNoiseFigure_dB
```

Logo, a coluna `snr` atual de `monitor.h:165` excede o escalar `10log10(snrPer.snr)` pela figura de ruído. Isso decorre da fórmula do trace, não de uma nova medição. A figura de ruído deve ser lida da interface receptora em runtime. `InterferenceHelper::CalculateSnr` inclui ruído térmico, figura de ruído e interferência (`interference-helper.cc:237–258`). `snrPer.snr` é calculado no início do payload (`:690–705`); o PER acumula os trechos com alterações da interferência. Portanto o escalar recuperado não é um SINR médio de todo o pacote nem determina sozinho o PER.

O módulo mantém separadas média aritmética de dBm, potência média linear convertida para dBm, média do ruído fornecido, diferença sinal/ruído do trace e escalar efetivo recuperado. Os dois tipos de média de potência têm interpretações diferentes. Todas essas estatísticas continuam condicionadas às recepções PHY bem-sucedidas, com contagem de amostras explícita. Não converter ausência de sinal em zero nem inventar RF para falhas não instrumentadas.

### Controle analítico do sinal

Para propagação LogDistance determinística, sem modelos adicionais, validar o sinal por distância com os parâmetros realmente instalados. Este fork adiciona um termo de frequência que não faz parte da fórmula LogDistance básica apresentada na [documentação ns-3](https://www.nsnam.org/docs/models/html/propagation.html). A implementação local é a referência da expressão concreta (`src/propagation/model/propagation-loss-model.cc:510–537`):

```text
d > d0:
signalExpected_dBm = TxPower_dBm + TxGain_dB + RxGain_dB
                     − ReferenceLoss_dB
                     − 10 × exponent × log10(d/d0)
                     − 21 × log10(PropagationFrequency_Hz / 900e6)
d <= d0:
signalExpected_dBm = TxPower_dBm + TxGain_dB + RxGain_dB
```

A ramificação próxima retorna potência sem a perda de referência: existe uma descontinuidade no limite. O helper reproduz esse comportamento para detectar divergência; não o corrige silenciosamente.

Com perfil original `exponent=3.76`, `ReferenceLoss=8 dB`, `d0=1m`, frequência da propagação 900 MHz, AP `TxPower=30 dBm/TxGain=3 dB/RxGain=3 dB`, STA `TxPower=0 dBm/TxGain=0/RxGain=0`, os controles calculados são:

| Distância | AP → STA | STA → AP |
|---|---:|---:|
| 100 m | −50,2 dBm | −80,2 dBm |
| 1.000 m | −87,8 dBm | −117,8 dBm |

Esses números são resultados da fórmula e entradas unitárias, não observações de alcance/entrega. Registrar separadamente `PropagationFrequency` e frequência central PHY: o centro esperado de 905 MHz não altera automaticamente o atributo de propagação de 900 MHz. Para validar no ns-3, um controle com dois nós, posição fixa e tráfego leve deve comparar o trace com a fórmula usando tolerância pré-fixada, por exemplo `1e-6 dB` em ambiente sem fading. Ajustar MCS/payload/interferência muda a entrega sem mudar necessariamente a potência recebida.

## Como integrar sem confundir camadas

| Evento | Uso permitido / limite |
|---|---|
| Geração do probe, ID estável | `Sent()`: denominador inclui perdas antes de transmissão física |
| Entrega ao destino pretendido do probe de um salto | `Received()`: deduplicação e deadline; confirmar origem/interface e hop único |
| `PhyTxBegin` de dados/probe | `Attempt()`: uma identidade por transmissão física real; separar duplicação de callback e excluir ACK/RTS/beacons |
| `MonitorSnifferRx` de probe identificado | `SuccessfulRf()`: sinal/ruído somente de quadros bem-sucedidos, inclusive retries se declarados como categoria |
| `MacTx` | Enfileiramento/aceitação no MAC; não é número de transmissões físicas |
| `TxOkHeader` / `TxErrHeader` | Resultado do MAC baseado em confirmação; não equivale automaticamente a PDR do receptor |
| `MacTxDataFailed` / `MacTxFinalDataFailed` | Falha intermediária/final por parceiro; sem ID de probe, não deduzir sozinho número de tentativas por probe |

Os hooks MAC são definidos em `regular-wifi-mac.cc:762–769`, `wifi-mac.cc:207–230` e `wifi-remote-station-manager.cc:328–343`. Uma recepção pode ocorrer e seu ACK se perder, causando retry e falha percebida pelo emissor apesar de entrega ao receptor. Por isso os dois lados devem ser medidos e exportados como indicadores distintos.

Um ID estável no payload/tag deve atravessar cópias/retries; `Packet::GetUid()` ou apenas sequência MAC de 12 bits não basta sem validar duplicação, re-enfileiramento, fragmentação e wrap. A identidade de tentativa física é diferente da identidade lógica. Para interpretar contadores de tentativas iguais a zero, o manifesto do adaptador precisa declarar se o hook está conectado: zero com hook ausente é ausência de instrumentação. O módulo não consegue adivinhar esse estado.

Para probes entre futuros candidatos ainda não associados, há trabalho adicional: o modo infraestrutura atual não permite simplesmente enviar sondagens IP de STA para qualquer vizinho e interpretá-las como um salto direto. Deve haver etapa de sounding/associação controlada ou outro mecanismo declarado. O monitor passivo de beacons ajuda a formar candidatos, mas não valida retorno UL. Uma rota passando pelo gateway não mede o enlace direto candidato→folha.

## Regra proposta para relays estáticos

1. Todos os nós permanecem imóveis. Definir candidatos e restringir sondagem ao conjunto plausível, sem calcular todos os pares automaticamente nem concluir qualidade pela fórmula de RSSI.
2. Após aquecimento, em uma etapa de calibração com parâmetros registrados, medir separadamente candidato↔gateway/pai e candidato↔filho, em ambas as direções. Exigir associação ao AP correto, acesso RAW válido, amostras suficientes e identidade consistente.
3. Usar como score conservador o **mínimo dos quatro `LQI_exp_percent`**: duas direções do backhaul e duas do acesso. Tratar candidato sem algum enlace medido como insuficiente. Não multiplicar PDRs como se os saltos fossem independentes.
4. Priorizar qualidade e capacidade de acrescentar cobertura. RSSI entra como contexto/critério de elegibilidade validado por MCS/PHY, sem valor universal ou obrigação de escolher a potência mais fraca. Um sinal mais fraco pode servir melhor se a entrega bidirecional medida for superior; ruído/interferência são campos separados.
5. Fixar desempate, por exemplo maior avanço espacial e depois ID do nó. Formar árvore enraizada sem ciclos, explicitando pai e razão da escolha. A quantidade de hops também afeta o objetivo e precisa de limite/regra identificados.
6. Congelar lista de relays e árvore após calibração. Os cenários R1 e R2 compartilham exatamente as mesmas posições, relays e relações pai/filho; muda somente a política RAW definida para a comparação. Não selecionar novamente cada cenário favorecendo um deles.
7. A qualidade da calibração não garante a mesma qualidade sob tráfego/interferência dos quatro cenários. Medir novamente no período experimental sem trocar a topologia; diferenciar seleção inicial de comportamento sob carga.

O score acima é uma proposta operacional verificável. A escolha de mínimo, tamanho/cadência de probes, população atendida e desempates deve ser congelada no protocolo antes das réplicas. Seleção dinâmica e mobilidade não estão incluídas nesta proposta.

## Entrega e verificações executadas

- `implementation/link-quality.h/.cc`: C++11 standalone, sem ns-3, RNG, relógio de parede ou dependências externas; contagem por enlace/interface/direção, prazo terminal, duplicatas, tentativas, RF e helper analítico.
- `tests/link-quality-check.cc`: entradas sintéticas destinadas apenas a regressão. Nenhum dado de pesquisa foi fabricado.
- Comando executado no Ubuntu/WSL: `bash tests/run-link-quality-check.sh`.
- Compilador: GCC 15.2.0; opções `-std=c++11 -Wall -Wextra -Werror -pedantic -O2`.
- Resultado: **64 verificações passaram**, incluindo direções/interfaces separadas, geração fora da janela, drenagem, deadline exclusivo, pending versus perda, duplicatas, IDs reutilizados, retries físicos, ausência/insuficiência de amostras, RF, figura de ruído, assimetria AP/STA, termo de frequência e ramificação da referência.

O executável fica em `validation/link-quality/link-quality-check`. Esses testes validam a lógica standalone e a aritmética documentada; ainda faltam o adaptador ns-3, controles RF de dois nós e validação de associação/RAW/probes na base original. `experimentalLqiDefined` é somente o gate interno de amostragem/identidade/prazo; a validade científica do modelo e do protocolo continua um gate externo obrigatório.
