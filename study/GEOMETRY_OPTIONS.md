# Geometria do primeiro cenário estrela

Este documento interpreta somente a geometria anotada na reunião: ambiente de 5 km × 5 km, matriz de 2.500 nós, arestas de 100 m e gateway no centro ou na ponta. As coordenadas são cartesianas, em metros, no domínio `[0, 5000] × [0, 5000]`; não são latitude e longitude. O cenário inicial usa estrela direta, um grupo RAW e nenhum relay.

## Contagem e extensão

- Uma matriz completa de **50 × 50 pontos contém 2.500 posições**, mas ocupa **49 intervalos de 100 m em cada eixo**, portanto mede **4.900 m × 4.900 m** entre os pontos extremos.
- Uma matriz completa com pontos nas duas bordas de uma extensão de **5.000 m**, a cada 100 m, precisa de **51 × 51 = 2.601 posições**.
- É perfeitamente possível colocar a matriz 50 × 50 dentro de um ambiente 5.000 m × 5.000 m. O tamanho do ambiente e a extensão entre os nós são medidas diferentes.

A contagem foi confirmada pelo pesquisador: **2.500 estações + 1 AP**, portanto 2.501 dispositivos. Foi preparada a opção C no canto `(0,0)`, mantendo 100 m mínimos inclusive para o AP. As outras opções abaixo registram alternativas geométricas, não decisões pendentes sobre a contagem atual.

## Opção A — 2.500 dispositivos, incluindo o gateway

É a alternativa que conserva uma matriz completa 50 × 50 e distância mínima de 100 m entre todos os dispositivos.

Posições da matriz:

`(x_i, y_j) = (50 + 100 i, 50 + 100 j)`, com `i, j = 0, ..., 49`.

Há margens de 50 m nas quatro bordas do ambiente. O AP **substitui** uma estação na matriz; não é adicionado por cima dela.

| Local do AP | Coordenada do AP | Estações | Menor distância AP–estação | Maior distância AP–estação |
|---|---|---:|---:|---:|
| Próximo ao centro | `(2450, 2450)` | 2.499 | 100 m | `2500 √2 ≈ 3535,53 m` |
| Ponta da matriz | `(50, 50)` | 2.499 | 100 m | `4900 √2 ≈ 6929,65 m` |

A matriz tem um número par de posições em cada eixo. Seu centro geométrico, `(2500, 2500)`, fica entre quatro pontos. Os quatro pontos igualmente próximos ao centro são `(2450, 2450)`, `(2450, 2550)`, `(2550, 2450)` e `(2550, 2550)`. Escolher um deles desloca o AP em aproximadamente 70,71 m em relação ao centro exato do ambiente. As quatro escolhas são equivalentes por simetria nesta geometria.

## Opção B — 2.500 estações em matriz + AP no centro exato

Se mantivermos todas as 2.500 estações nas posições da opção A e adicionarmos o AP em `(2500, 2500)`, teremos 2.501 dispositivos. As estações continuam espaçadas em 100 m, porém as quatro mais próximas ficam a **`50 √2 ≈ 70,71 m` do AP**. Isso viola a distância mínima global de 100 m, caso a regra inclua o AP.

A maior distância AP–estação nesta opção é `2450 √2 ≈ 3464,82 m`. Esta opção só atende à anotação se os 100 m forem exigidos **entre estações**, com uma exceção explicitamente aceita para o gateway.

## Opção C — 2.500 estações em matriz + AP separado, na borda

Uma solução sem exceção de espaçamento é deslocar a matriz para:

`(x_i, y_j) = (100 + 100 i, 100 j)`, com `i, j = 0, ..., 49`.

As estações ocupam `x = 100 ... 5000` e `y = 0 ... 4900`. O AP fica fora da matriz, mas dentro do ambiente. São 2.500 estações + 1 AP e a menor distância entre quaisquer dispositivos permanece em 100 m.

| Local do AP | Coordenada do AP | Menor distância AP–estação | Maior distância AP–estação |
|---|---|---:|---:|
| Canto do ambiente | `(0, 0)` | 100 m | `√(5000² + 4900²) ≈ 7000,71 m` |
| Meio da borda esquerda | `(0, 2500)` | 100 m | `√(5000² + 2500²) ≈ 5590,17 m` |

“Na ponta” precisa distinguir canto e meio da borda: as distâncias e a cobertura geométrica são diferentes.

## Opção D — 2.500 estações + AP no centro exato, com um ajuste na matriz

É possível manter 100 m mínimos e o AP em `(2500, 2500)` usando a grade `x, y = 0, 100, ..., 4900`, substituindo a estação em `(2500, 2500)` pelo AP e acrescentando uma estação em `(5000, 2500)`.

Resultado: 2.500 estações + 1 AP, distância mínima global de 100 m e maior distância AP–estação de `2500 √2 ≈ 3535,53 m`. Entretanto, as estações deixam de formar uma matriz completa 50 × 50: há a posição do AP e um ponto adicional na borda. Esse ajuste deve constar do protocolo, caso seja escolhido.

## Critério para o primeiro controle

Para uma matriz completa 50 × 50, com distância mínima global de 100 m, a opção A é a implementação mais direta quando os 2.500 dispositivos incluem o AP. Se a reunião especificou 2.500 **estações**, a opção C conserva a matriz e o espaçamento com AP na borda; um AP no centro exato exige a exceção da opção B ou a mudança explícita da opção D.

As distâncias acima são limites geométricos, não garantias de alcance ou associação. No primeiro experimento, devem ser registrados separadamente a posição de cada estação, sua distância ao AP e o resultado da associação. As estações que não se associam permanecem no cenário e entram nas estatísticas de cobertura; não devem ser removidas para melhorar a taxa de entrega.

A opção inicial está registrada em `prepared/star-2500-corner`: 2.500 estações, AP adicional no canto e distância mínima global de 100 m. A comparação posterior entre centro e ponta deve manter a mesma população de estações, quando possível, e registrar qualquer mudança necessária na geometria.
