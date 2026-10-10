# Histórico de avanços — Wi-Fi HaLow / ns-3

## 09/10/2026 — Preparação da topologia

- Organização de uma topologia planejada com 2.500 estações e um gateway.
- Área de estudo de 5 km x 5 km.
- Distância mínima geométrica prevista de 100 metros.
- Preparação dos arquivos de posições, mapa e verificações.
- Organização das implementações e dos testes.

### Evidências
- prepared/star-2500-corner/topology.json
- prepared/star-2500-corner/layout.svg
- prepared/star-2500-corner/geometry-verification.json
- prepared/star-2500-corner/raw-preflight.json

### Limitação
A preparação geométrica não comprova a associação e a comunicação simultânea das 2.500 estações no ns-3.

## 10/10/2026 — Organização do repositório

- Criação de um repositório Git independente.
- Organização do código, das ferramentas e dos testes.
- Preservação da base original utilizada como referência.
- Preparação da documentação para compartilhamento.

## Próximas etapas

- Integrar as posições reais ao cenário ns-3.
- Verificar a criação dos 2.501 nós no simulador.
- Validar associação Wi-Fi HaLow e configuração RAW.
- Executar testes de tráfego UL e DL.
- Registrar perdas, atrasos e indicadores por estação.

## Repositório de referência

https://github.com/pablopereira70/relay-halow-tree
