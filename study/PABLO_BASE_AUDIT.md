# Auditoria da base Pablo para o cenário de 2.500 estações

Data: 9 de outubro de 2026. Fonte: ZIP indicado pelo pesquisador, SHA-256 `9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6`. Esta é uma revisão de código e do preparo geométrico, não uma execução ou validação completa do ns-3.

## Escolha do cenário

`scratch/test` é o cenário estrela original; o programa é construído como target `test` pelo `wscript:727–733`. `scratch/test++` cria relays e folhas, incluindo `Nrelays × NleafsPerRelay` na versão atual (`test++/s1g-test-tim-raw.cc:1184–1194`). O README de `test++` descreve uma versão antiga e não serve como comprovação da topologia atual.

O cenário estrela cria N estações e um AP (`test/s1g-test-tim-raw.cc:1205–1206`), usa `/16` (`:1365`) e declara arrays para 8.000 estações. Esses pontos não impedem 2.500; a rede `/24` da arquitetura relay não deve ser atribuída a este target.

## Bloqueios do primeiro experimento

| Código | Evidência na base | Consequência / demanda |
|---|---|---|
| População | `test/s1g-test-tim-raw.cc:1171`, `Nsta=NRawSta` após leitura RAW | Separar número de estações físicas do conjunto escalonado pelo RAW |
| Geometria | `test/s1g-test-tim-raw.cc:1308`, `UniformDiscPositionAllocator` | Substituir pelo arquivo de posições da matriz validada |
| Início das aplicações | `test/s1g-test-tim-raw.cc:339–367`, exige `GetAssocNum()==Nsta` | Uma estação fora de alcance impede todo o tráfego. Usar janela fixa e resultados por estação, sem excluir silenciosamente as que falham |
| RAW / páginas | `rps.cc:124–135` e `sta-wifi-mac.cc:1525–1541` | Página de 2 bits; início/fim locais de 11 bits. AIDs 1–2500 exigem duas páginas. `2500 & 0x7ff=452`; não produzir configuração truncada |
| AIDs globais | `ap-wifi-mac.cc:768–771` | Identidade é derivada do MAC, não reatribuída de forma contígua aos associados. Confirmar MAC/AID/página por posição |
| Agenda AP | `ap-wifi-mac.cc:1251–1270` | AP usa limites locais como globais, sem reconstruir página×2048+local; slot usa 10 bits e STA usa 11. Corrigir/testar antes de multipágina |
| Página TIM | `ap-wifi-mac.cc:1056–1057`; `sta-wifi-mac.cc:416–423` | Página fixa e atualização comentada; outra página pode dormir. Duas linhas de RAW não resolvem isso por si só |
| Associação parcial | `ap-wifi-mac.cc:873,899,946,1231,1270` | `find(...)->second` sem guard em mapas potencialmente esparsos. Exigir regressão com estações ausentes/fora de alcance |
| Cópia RPS | `rps.h:103–104`; `rps.cc:36–52,314,393` | O defeito de ponteiro interno continua nesta base. Confirmar e corrigir isoladamente sobre os bytes deste ZIP, sem copiar o restante do estudo anterior |
| TIM | `tim.cc:148,206–226`; `tim.h:160–161` | Campos não inicializados, ponteiro interno e escrita com checagem tardia de limite; regressões próprias de cópia/serialização/fronteiras necessárias |

Correção da interpretação inicial: um **intervalo limitado a uma página** não representa todos os AIDs. Isso não demonstra impossibilidade de representar um grupo global: existe codificação especial de grupo para todas as estações, que o fork não interpreta corretamente. A página 0 contém locais 1–2047; a página 1 contém locais 0–452. A definição humana exige um único grupo para todas; rotação de páginas não é a política escolhida. Não aumentar silenciosamente os campos de onze bits. A análise da representação especial e dos bloqueios de sono/DL está em `RAW_GLOBAL_IMPLEMENTATION.md`.

## Parâmetros RF realmente usados

O cenário instala estações com 0 dBm e ganhos 0 dB (`test/...cc:1227–1228`) e AP com 30 dBm e ganhos +3 dB (`:1262–1267`). A propagação instalada usa expoente 3,76, referência de 8 dB a 1 m (`:1209–1211`), mesmo existindo outro default na classe Configuration. Esses números são configuração do código; não foram apresentados como calibração empírica do ambiente de 5 km.

`Configure80211ah` começa em 900 MHz (`yans-wifi-phy.cc:1005–1008`); o centro reportado soma 5 MHz por canal (`:501–503`). Com canal padrão 1, o centro esperado é 905 MHz. Confirmar esses valores em runtime, em vez de registrar somente o atributo inicial.

Receber beacon do AP mais potente não comprova retorno UL nem associação. Medir separadamente: recepção de beacon, associação ao AP esperado, participação RAW válida e entrega única por direção. Posição fora de alcance é resultado do experimento; ausência de autorização RAW ou defeito de índice não é evidência de alcance RF.

## Qualidade e ordem espacial

O fork fornece `MonitorSnifferRx`, sinal/ruído e traces de drops (`wifi-phy.cc:94–108`; `wifi-phy.h:1672`). Não foi encontrada métrica LQI pronta na implementação Wi-Fi. Um indicador de qualidade futuro deve ter denominador e janela explícitos: entrega, retries, erro de pacote, SINR ou outra medida validada. RSSI somente de quadros recebidos com sucesso tem viés e não pode substituir todas as tentativas.

Ao instalar STAs em ordem de linhas, os primeiros 2.047 AIDs e os últimos 453 ficam em regiões diferentes. Exportar esse mapeamento e, se necessário, estudar permutações de AID/posição sem mudar a geometria; de outro modo, paginação pode parecer efeito espacial.

## Estado desta preparação

Arquivo de posições e SVG preparados; 2.500 estações mais um AP, distância mínima global 100 m, domínio 5 km × 5 km. O código importado permanece igual aos hashes de importação. Nenhuma simulação ns-3 foi iniciada e nenhum resultado de cobertura/PDR/tempo de execução foi fabricado. O preflight registra a política global já confirmada e mantém sua implementação/validação como pendência técnica.
