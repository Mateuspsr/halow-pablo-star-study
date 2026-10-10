# Implementação de um RAW para todas as estações

Data: 9 de outubro de 2026. Base: `relay-halow-tree-main.zip`, SHA-256 `9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6`. Esta nota revisa a limitação identificada na auditoria inicial; nenhuma alteração foi aplicada ao código importado. O helper isolado em `implementation/raw-membership.h` e seu teste são preparação de implementação, não uma simulação ns-3 validada.

## Requisito humano

Os quatro cenários são estáticos durante a medição:

1. Estrela, um único grupo RAW para todas as 2.500 STAs.
2. Estrela, mais de um grupo RAW, com a partição cobrindo todas as STAs.
3. Vários relays, um único grupo RAW comum a toda a rede, incluindo as interfaces STA dos relays.
4. Vários relays, exatamente um grupo RAW por relay; política do gateway/backhaul ainda precisa ser definida.

Alternar as páginas em diferentes beacons não substitui os requisitos 1 e 3. Um grupo pode conter vários slots: quantidade de grupos e quantidade de slots devem ser parâmetros e resultados distintos. Um slot é um controle inicial possível, não uma exigência do pesquisador.

## Resultado da investigação: existe uma exceção para todas as STAs

O [rascunho primário IEEE P802.11ah/D0.1, maio de 2013](https://mentor.ieee.org/802.11/dcn/13/11-13-0500-00-00ah-proposed-tgah-draft-amendment.pdf), página impressa 78, define uma exceção: RAW Group inteiramente zero permite acesso de todas as STAs. Os campos de página/início/fim continuam com 2/11/11 bits. A página 131 define o mapeamento não restrito a paged STAs por `(AID + offset) mod númeroDeSlots`; o offset deriva de dois bytes do FCS do beacon. O texto foi localizado na indexação da fonte IEEE; a tentativa de abrir o PDF completo retornou HTTP 418. Não foi realizada verificação integral do documento final IEEE 802.11ah-2016.

O [dissector RPS oficial do Wireshark](https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-ieee80211.c), consultado em 9/10/2026, também reconhece o grupo zero como acesso para todas as STAs (`dissect_rps`, linhas 24013–24020 na revisão consultada). Ele lê o grupo somente quando a indicação de presença está ativa. Seu RAW Control separa tipo, opções e presença; a indicação do grupo é `0x20`. Essa é evidência de implementação atual do formato, não certificação normativa deste fork.

Portanto, **a amplitude de onze bits limita o intervalo explícito de uma página, mas não prova impossibilidade de representar um grupo para todas as STAs**. A afirmação inicial deve ser lida como ausência de implementação dessa exceção no caminho AP/STA do fork. Não é necessário alargar o intervalo ou alternar páginas para expressar o pertencimento global.

## O formato importado não pode ser rotulado como formato final validado

O fork serializa cada assignment como seis bytes: um byte de controle, dois de definição de slots e três de grupo (`rps.cc:289–314`). `GetRawAssigmentObj` assume seis bytes e extrai apenas os três bits baixos de controle (`rps.cc:345–368`); não interpreta a presença opcional do grupo. `GetNumberOfRawGroups` divide o tamanho por seis (`rps.cc:399–402`).

Além disso, o fork coloca Slot Format em `0x8000` e Cross Slot Boundary em `0x4000` (`rps.cc:196–215`), enquanto o dissector atual usa `0x0001`/`0x0002`. Não basta mudar `RawControl=0` para `0x20` e anunciar conformidade: seria necessária auditoria de serialização, parsing e semântica de todas as variantes.

Perfil mínimo proposto: `legacy-fork-global-zero-experimental-v1`. Mantém o formato de seis bytes da base e implementa explicitamente a exceção de grupo zero. Identificar o perfil em cada execução, guardar o original e executar comparações de regressão. Uma migração ao formato final é trabalho separado.

## Contrato do helper de pertencimento

O helper deve distinguir intervalo de página e wildcard:

- Campo de grupo válido: inteiro de 24 bits; rejeitar bits superiores em vez de truncar silenciosamente.
- STA de dados válida: associada ao AP esperado e AID entre 1 e 8191. AID zero e sentinela 8192 do fork ficam fora da participação de dados; acesso de associação é política de gestão separada.
- Grupo exatamente zero: pertence qualquer STA válida do domínio, independentemente de sua página.
- Grupo não zero: pertence se a página do AID coincide com a página indicada e seus onze bits locais estão no intervalo inclusivo início/fim. Rejeitar início maior que fim.
- AID 2048, cuja parte local é zero na página 1, é diferente de AID global zero.
- O helper de um AP verifica pertencimento no BSS desse AP. A exceção não autoriza receber beacons de qualquer BSSID nem associar-se a outro AP.

API conceitual: `IsAllStationsGroup(group24)`, `RawMembership(aid, group24, associated)` e `RawSlotIndex(aid, slots, offset)`. A implementação concreta pode usar nomes equivalentes. Um domínio global entre BSSs requer estrutura própria, descrita abaixo; não inserir essa coordenação silenciosamente no helper local.

Para o perfil experimental não paginado, usar AID completo no índice `(aid + offset) % slots`, calculado com o mesmo helper pelo AP e pela STA. Tratar soma em largura suficiente para evitar overflow. O fork usa máscaras diferentes no AP (`0x03ff`) e na STA (`0x07ff`) e força offset zero. Registrar `slotMapping=full-aid`, `offsetPolicy=fixed-zero-control` quando usar esse controle determinístico. Offset zero não implementa a regra FCS; não alegar que o helper completa conformidade. Uma alternativa de comparação com `legacy-low11` precisa de outro perfil explícito, aplicado aos dois lados.

## Integração AP/STA/TIM/sono ainda necessária

| Região da fonte importada | Mudança a projetar e validar |
|---|---|
| `rps.cc:36–52,268–273,314,393` e `rps.h:103–104` | Corrigir a propriedade dos bytes ao copiar `RPS` antes de qualquer novo comportamento. `RPSVector` atualmente clona objetos com cópia padrão que conserva ponteiros internos da origem. |
| `ap-wifi-mac.cc:1251–1270` | No wildcard, percorrer os associados reais, com seus AIDs completos; em intervalo, selecionar a mesma população por helper. Nunca percorrer 0..0 esperando representar todos. Reconstruir a página nos intervalos. Usar o mesmo mapeamento de slot da STA. |
| `ap-wifi-mac.cc:873,899,946,1231,1270` | Remover dereferenciamento de `find(...)->second` ausente; associação parcial e AIDs esparsos são condições normais no domínio de 5 km. |
| `sta-wifi-mac.cc:1525–1547` | Aplicar wildcard antes do filtro de página, calcular slot pelo helper e armazenar corretamente o grupo/slot correspondente. O acesso continua condicionado à associação ao BSSID esperado. |
| `sta-wifi-mac.cc:1552–1554` e `S1gTIMReceived:373–590` | O processamento TIM atual agenda sono de outra página (`416–423`) e de STAs sem DL indicado (`451–458`). Em RAW global não restrito a paged STAs, informação TIM não pode retirar a participação UL autorizada. Eventos de sono devem respeitar a próxima janela RAW global. |
| `sta-wifi-mac.cc:806–837` | `RawSlotStartBackoff` só acorda se existe pacote UL na fila. Para entregar DL a membro com UL vazio, a política deve assegurar recepção no slot correspondente, considerando TIM/DL ou uma política experimental de escuta explícita. |
| `ap-wifi-mac.cc:992–1002`, `HasPacketsToSubBlock:889–905` | AP marca todos como dormindo e só a página TIM servida desfaz parte desse estado. Sincronizar autorização de envio DL e estado de escuta para todos os membros do slot. Não marcar acordado no AP se a STA permaneceu dormindo. |
| `S1gBeaconReceived:1288–1343`, `GoToSleep:654–674`, `InsideBackoff:849–859` | Testar fronteiras de slot/beacon, cancelamento de eventos anteriores e conclusão TX/RX/ACK. Corrigir pertencimento isoladamente não valida sono ou retransmissões. |
| `extension-headers.cc:84–100,151–165`, `tim.h:160–161`, `tim.cc:148,206–226` | Validar propriedade de memória TIM, inicialização, limite de bitmap e parsing. Não copiar um TIM por página e assumir que informa todas as páginas. |
| `scratch/test/s1g-test-tim-raw.cc:165–175,339–367,1171` | Um wildcard zero não soma 2500 no contador derivado do intervalo. A população deve vir da configuração física, separada da quantidade associada/escalonada. Aplicações precisam de janela fixa e contabilidade individual. |

Não foi preparado patch MAC candidato: o contrato completo de sono, DL, grupo global e páginas ainda exige integração e verificação. O helper isolado serve como teste unitário de regras, sem provar funcionamento de rede.

## Domínio global entre relays: cenários 3 e 4 são diferentes

No cenário 1, wildcard local do único AP cobre o domínio desejado de todas as STAs associadas. No cenário 3, cada relay AP introduz outro BSS. Um wildcard em cada BSS, mesmo com duração idêntica, continua sendo vários grupos locais; não satisfaz a definição humana de um único grupo comum.

Para representar o cenário 3 no simulador, a extensão deverá ter um único objeto de política/domínio global: identificador de grupo, conjunto explícito de participantes por interface STA, época de beacon, início e fim absolutos de cada janela, slots e política de offset. Os APs/relays disseminam ou recebem a mesma política, e seus autorizadores de transmissão consultam o mesmo domínio. Medir que os participantes efetivamente seguem os mesmos limites temporais. Cada MAC continua responsável pela associação ao BSS correto; AID é identidade local de BSS, não chave única suficiente do domínio inteiro.

A política global precisa abranger o encaminhamento pelos relays, os envios AP/DL e a distinção entre dados e quadros de gestão/ACK. A coordenação não surge automaticamente do `RpsIndex` estático compartilhado (`ap-wifi-mac.cc:932`) nem de beacons independentes. Se o controlador entregar uma agenda por acesso direto a objetos ns-3, registrar coordenação idealizada e seu overhead não modelado; isso é extensão de pesquisa, não sinalização padronizada comprovada.

No cenário 4, cada relay possui seu domínio de grupo e membros ligados a ele. Pode haver janelas coincidentes e interferência no mesmo canal; coincidência temporal não une os grupos. Registrar o número efetivo de grupos, o serviço de backhaul/gateway e os canais. Manter topologia, papéis e canais iguais na comparação com cenário 3; alterar isolamento de canais cria outro fator experimental.

## Verificações antes dos resultados de pesquisa

1. Helper: wildcard para AIDs 1, 1023, 1024, 2047, 2048, 2500 e 8191; rejeitar 0, 8192, não associado e grupo acima de 24 bits. Intervalo página 1 com locais 0..452 inclui 2048 e 2500 e exclui 2047/2501.
2. Slots: 1, 2, 3 e 7 slots; offset zero e não zero; AP/STA concordam na página e no slot; testar AIDs iguais nos onze bits, mas de páginas distintas. Não testar só divisores de 2048, que escondem diferenças de máscara.
3. Serialização/propriedade: cópia e atribuição RPS/TIM, destruição da origem, round trip dos seis bytes do perfil e distinção entre wildcard zero e intervalo local começando em zero.
4. Rede pequena: associação parcial, membros das páginas 0 e 1 no mesmo grupo/beacon, UL puro, DL com UL vazio, bidirecional e escuta por slot. Exportar tempos reais de autorização e sono, além de entrega única/duplicações.
5. Regressão: perfil original identificado e corrigido; intervalos normais não ganham wildcard por engano. Repetição exata, invariância a caminhos de saída e accounting íntegro.
6. Múltiplos BSSs: dois relays com participantes explícitos; cenário 3 reporta um domínio único, cenário 4 reporta dois grupos locais. Associação ao AP correto, mesma janela absoluta, encaminhamento e orçamento de airtime documentados.
7. Escala: somente depois dos controles, executar 2500 estações, com nenhum AID excluído por página, mantendo os ausentes de associação como resultado individual. Limiar de perda acima de 5% só caracteriza degradação depois de validado o serviço RAW.

Estado: caminho de implementação encontrado e regras testáveis definidas; source original intacto; nenhuma alegação de simulação executada, cobertura medida ou compatibilidade integral com o padrão final.
