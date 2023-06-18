---
title: "Relatório T2"
output: pdf_document
---

> CI1316 1º semestre 2023
> Lucas Müller
> GRR20197160
> Tiago Serique Valadares
> GRR20195138

# Trabalho 2

## Introdução

Neste trabalho, implementamos um algoritmo de broadcast em MPI e o comparamos com a versão utilizada no MPI nativo. O objetivo foi testar o desempenho do algoritmo em diferentes tamanhos de mensagens, sendo eles 4KB e 16KB. Optamos pela utilização da Split Binary Tree como topologia interna do MPI, devido à sua eficiência nesses casos.

Além de distribuir as mensagens de acordo com a nossa nova topologia de árvore, implementamos as etapas de "quebra de mensagens" em metades, seguindo o procedimento da Split Binary Tree, e a "troca de metades" na etapa final.

Em cada etapa do algoritmo, cada nó recebeu uma metade da mensagem, e na etapa final ocorreu a troca de metades entre os nós.

## Implementação

A função `my_Bcast_rb()` implementa um algoritmo de broadcast em MPI (Message Passing Interface) usando uma topologia de árvore binária dividida. O objetivo é distribuir uma mensagem a partir de um nodo raiz para todos os outros nodos em um comunicador MPI.

A função recebe os seguintes parâmetros:
- `buffer`: Um ponteiro para o buffer contendo a mensagem a ser transmitida.
- `count`: O número de elementos no buffer.
- `datatype`: O tipo de dado dos elementos no buffer.
- `root`: O rank do nodo raiz que irá enviar a mensagem.
- `comm`: O comunicador MPI que define o grupo de nodos envolvidos na comunicação.

A implementação do algoritmo é baseada na ideia de que cada nodo recebe uma metade da mensagem em cada etapa e, na etapa final, ocorre a troca das metades entre os nodos.

A função começa obtendo o rank físico do nodo atual no comunicador MPI e o número total de nodos no comunicador. Em seguida, é calculado o rank lógico do nodo em relação ao nodo raiz.

Se o nodo não for o nodo raiz, ele espera para receber sua metade da mensagem. O nodo fonte de onde ele deve receber a mensagem é calculado com base no rank lógico. Dependendo do valor do rank lógico, o nodo recebe a metade esquerda ou direita do buffer.

Em seguida, ocorrem as etapas de envio das metades do buffer. O número de etapas é determinado pelo logaritmo base 2 do número total de nodos. O nodo verifica se ele está na posição correta para enviar sua metade do buffer na etapa atual. Se sim, ele calcula o destino lógico e físico do envio com base no rank lógico e no nodo raiz. O nodo utiliza a função `MPI_Send` para enviar sua metade do buffer para o destino correto. O nodo raiz e os nodos com ranks maiores do que o último nodo par não enviam suas metades.

Após as etapas de envio, ocorre a etapa final de completar a outra metade do buffer a partir do nodo vizinho que possui a outra metade. Se o nodo atual for par, ele envia sua metade para o nodo vizinho e recebe a outra metade desse mesmo nodo vizinho. Se o nodo atual for ímpar, ele recebe a metade do buffer do nodo vizinho e envia sua metade de volta. Essa etapa final também lida com o último nodo ímpar, que envia sua metade para o próximo nodo. O último nodo par não executa nenhuma ação nessa etapa.

No final da função, o broadcast é concluído e a mensagem é transmitida de forma eficiente para todos os nodos no comunicador MPI, seguindo a estratégia da árvore binária dividida.

## Experimento

O experimento foi realizado em um cluster Xeon com as seguintes especificações:

- 18 nós Xeon;
- Cada nó possui 2 processadores;
- Cada processador possui 4 núcleos;
- O Xeon utilizado não possui suporte para hyperthreads;
- Portanto, cada nó pode executar até 8 processos MPI sem oversubscribe.

O objetivo do experimento foi avaliar o desempenho do algoritmo de broadcast implementado em comparação com a versão nativa do MPI, levando em consideração as restrições de hardware do cluster Xeon utilizado.
