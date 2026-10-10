# Comparação dos quatro cenários HaLow — base do Pablo

Base explicitamente indicada pelo pesquisador: `relay-halow-tree-main.zip`, SHA-256 `9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6`. A importação do arquivo original não executou instruções nem scripts do arquivo recebido. Links simbólicos de documentação e arquivos gerados/nomes não portáveis ficaram registrados como não materializados, preservados no ZIP original.

Esta pasta usa a nova base do Pablo. O ponto de partida de estrela é `source/relay-halow-tree-main/scratch/test/s1g-test-tim-raw.cc`; `scratch/test++` é a arquitetura com relays. O código do modelo importado não foi alterado nesta preparação.

## Preparação concluída

O pesquisador confirmou **2.500 estações mais um gateway**, total de 2.501 dispositivos. Foi preparada a opção de gateway na ponta, autorizada nas anotações da reunião:

- Ambiente `[0,5000] × [0,5000]` m.
- Gateway em `(0,0)` m; nenhum relay.
- Matriz completa 50 × 50 de estações: `x=100,200,…,5000`, `y=0,100,…,4900` m.
- Distância mínima de 100 m entre quaisquer dois dispositivos, incluindo o gateway.
- Distância ao gateway entre 100 m e aproximadamente 7.000,714 m. A diagonal de um quadrado de 5 km é maior que 5 km.

`prepared/star-2500-corner/topology.json` contém todas as posições e a distribuição **prevista**, ainda não observada, de AID/página seguindo a ordem de instalação do cenário oficial. `layout.svg` mostra somente geometria; não apresenta alcance ou entrega como resultados. A geometria foi verificada independentemente nas 3.126.250 duplas de dispositivos.

Para reproduzir em uma pasta nova:

```powershell
& 'C:\Users\mateu\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' tools/prepare_star.py --output prepared/NOVO_NOME
```

## Quatro cenários confirmados

| ID | Topologia estática | Política RAW |
|---|---|---|
| S1 | Estrela | Um único grupo para todas as estações |
| S2 | A mesma estrela | Mais de um grupo; 2 e 3 como subníveis candidatos |
| R1 | Rede com vários relays | Um único grupo global para toda a rede |
| R2 | A mesma rede com relays | Exatamente um grupo por relay |

A definição humana foi registrada em `study/FOUR_SCENARIO_CONTRACT.json`. Alternância de páginas não substitui S1/R1. Posições, papéis e rotas planejadas são fixos durante a medição; a seleção por qualidade/RSSI, se usada, precede e fica congelada para o par R1/R2. O método da comparação está em `study/FOUR_SCENARIO_METHOD.md`.

## Implementação e execução científica

Nenhuma simulação ns-3 de 2.500 estações foi iniciada nesta preparação. O arquivo `raw-preflight.json` identifica a pendência técnica: implementar e validar o grupo global já solicitado. Um intervalo de página de onze bits não contém os AIDs 1–2500; colocar 2500 no fim truncaria o valor para 452. Porém, existe representação especial de RAW para todas as estações. O fork não a trata como tal; a investigação está em `study/RAW_GLOBAL_IMPLEMENTATION.md`.

`implementation/raw-membership.h` implementa isoladamente a semântica experimental do grupo global, com pertencimento de AIDs associados de todas as páginas e atribuição de slot pelo AID completo. Seus testes passaram, mas o módulo ainda não está conectado ao ns-3. AP/STA/TIM/sono precisam de adaptação e testes de UL/DL, inclusive DL sem fila UL. O perfil de serialização legado exige identificação própria; não se afirma conformidade completa com o padrão final.

O cenário oficial também inicia as aplicações somente após todas as estações se associarem, o que impede medir “o gateway alcança quem conseguir” se qualquer estação estiver fora de alcance. A adaptação precisará de janela fixa de associação e contabilidade individual, com população física independente da configuração RAW.

As correções antigas do estudo de relays não foram transplantadas. A auditoria [PABLO_BASE_AUDIT.md](study/PABLO_BASE_AUDIT.md) identifica quais problemas também existem nesta base, com referências ao código. A agenda da reunião está em [MEETING_PROTOCOL_20261009.md](study/MEETING_PROTOCOL_20261009.md); geometria alternativa em [GEOMETRY_OPTIONS.md](study/GEOMETRY_OPTIONS.md). O RSSI implementado em `scratch/test++/monitor.h` precisa de validação por enlace/interface/direção e ainda não está instalado na estrela. A revisão e a proposta de qualidade estão em `study/LQI_RSSI_VALIDATION.md`.

## Aceleração

A avaliação de CPU/GPU fica em `study/GPU_FEASIBILITY.md`. Aceleração é uma etapa própria, após definir o modelo válido e medir o perfil de execução. Este preparo não alterou o motor ns-3, instalou bibliotecas GPU ou executou um benchmark de desempenho.

