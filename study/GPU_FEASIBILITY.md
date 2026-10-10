# Viabilidade de CPU/GPU para a estrela do Pablo

Data: 9 de outubro de 2026. Avaliação de código, documentação primária e inventário local; não é benchmark nem implementação de aceleração.

## Situação observada

O arquivo fornecido contém o fork legado ns-3.23 com módulos atualizados para 3.25. O cenário estrela usa Yans Wi-Fi e o motor de eventos do ns-3. Não foi encontrada integração de CUDA, OpenCL ou SYCL nos caminhos de build/modelo examinados. A documentação oficial descreve o motor padrão como execução sequencial de eventos em uma única thread: [Events and Simulator](https://www.nsnam.org/docs/manual/html/events.html). Essa documentação atual explica o mecanismo; a inspeção da base recebida é a referência da implementação concreta.

Assim, colocar parte desta simulação na GPU exige escolher operações e implementar uma integração. Alterar uma opção de compilação ou ativar MPI não transforma automaticamente a MAC/PHY HaLow em execução GPU.

Inventário local obtido por CIM: **Intel Iris Xe Graphics**, driver `32.0.101.7085`, aproximadamente 32 GB de RAM e 12 processadores lógicos. O Ubuntu/WSL observado expõe aproximadamente 15,5 GiB de memória. Esses valores descrevem esta máquina, não o ambiente do professor ou de um servidor. Não foi observado adaptador NVIDIA no inventário consultado. CUDA requer GPU NVIDIA compatível segundo o [guia de instalação da NVIDIA](https://docs.nvidia.com/cuda/cuda-installation-guide-microsoft-windows/index.html). O fato de a GPU ser Intel não impede toda computação GPU: o [fluxo de offload Intel](https://www.intel.com/content/www/us/en/docs/oneapi/programming-guide/2025-1/gpu-offload-flow.html) descreve SYCL com backends Level Zero ou OpenCL. Esses caminhos ainda precisam de verificação de driver/runtime, ferramenta e kernels específicos; não foi confirmado funcionamento de runtime GPU no WSL local.

## Caminhos e alcance

| Caminho | O que acelera | Situação para este estudo |
|---|---|---|
| Runs independentes em processos CPU | Tempo total de uma campanha | Primeiro candidato após medir RAM/tempo de um run; mantém cada simulação e seu RNG isolados |
| MPI dentro de uma simulação | Partições distribuídas sincronizadas | A documentação ns-3 limita a divisão padrão a enlaces ponto a ponto e cria a topologia completa em cada rank. Não resolve diretamente a estrela Yans nem garante reduzir sua RAM: [MPI for Distributed Simulation](https://www.nsnam.org/docs/models/html/distributed.html) |
| Pré-cálculo de geometria/perda estática | Cálculos repetidos de distância/logaritmo | Candidato a perfil e experimento; testar primeiro cache/vetorização CPU. Só é equivalente quando mobilidade, propagação e parâmetros justificam reutilizar o valor |
| Kernel GPU numérico específico | Lote grande de operações independentes | Pesquisa e integração adicionais; a CPU continua responsável pelos estados, eventos e sincronização necessários |
| Análise ou mapas offline em GPU | Pós-processamento | Não acelera a execução MAC/PHY original; medir como etapa separada |

Com 2.501 dispositivos, a matriz completa de pares tem 6.255.001 entradas, cerca de 50 MB se cada entrada usar um `double`. Esse é um cálculo de tamanho, não consumo observado nem evidência de que o pré-cálculo será o gargalo. Antes de implementá-lo, medir quanto do tempo é gasto em propagação versus agenda de eventos, colisões, associação, filas e traces. Cópia de dados CPU/GPU, inicialização e sincronização podem consumir o ganho de um kernel rápido.

## Plano de avaliação

1. Definir e validar o RAW/paginação e a referência de associação/tráfego. Um modelo inválido acelerado continua inválido.
2. Medir um run CPU em build identificado, com tempo de parede, user/system, pico RSS, progresso de tempo simulado e nível de tracing fixo.
3. Fazer perfil de funções e estimar a fração máxima acelerável; distinguir custo de um run do total de réplicas.
4. Comparar cache/pré-cálculo CPU com o mesmo algoritmo original e preservar resultados.
5. Só testar GPU se houver lote suficiente e hardware/backend compatíveis. Medir ponta a ponta, incluindo transferência e sincronização.
6. Verificar seed/run, consumo de números aleatórios, precisão/arredondamento, ordem de eventos e decisões de recepção. Valores numéricos próximos de um limiar podem mudar ACK, retry e entrega; equivalência precisa de tolerâncias e invariantes pré-definidos.

Não houve instalação de SDK/driver, troca do motor, campanha de 2.500 nós ou medição de speedup nesta preparação. A recomendação inicial é perfil CPU e replicações independentes com orçamento de memória; GPU fica como desenvolvimento de módulo específico condicionado ao perfil.
