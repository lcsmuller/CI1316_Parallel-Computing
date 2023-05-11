ci1316/ci316  - PROGRAMAÇÃO PARALELA - 
1o semestre de 2023    
por W.Zola/UFPR

Lab 1: SomaDePrefixos-com-PThreads   (versão 1)  
----------------------------------
Histórico:
- v1.0 a versão inicial
  
Data do enunciado: 10/maio/2023
Data da entrega Lab1: .../maio/2023 (será aproximadamente 1semana e meia!)


------- PASSOS PARA FAZER ESSE TRABALHO: -----------
O professor vai fornecer: 
 - o programa main completo
 - o script para rodar os experimentos
 - um Makefile exemplo do professor
 - uma planilha exemplo para voce rodar seus experimentos
   e incluir seus dados na pagina dadosV2
   ao incluir os seus dados a planilha já tem os gráficos
   como incluir?
   * compile com comando make
   * rode o script do professor chamado roda-v2.sh na sua máquina
     A MÁQUINA DEVE TER NO MÍNIMO 4 CORES E 8GB DE MEMÓRIA
     (voce DEVE ter um mínimo de processos rodando para não atrapalhar suas medidas)
     assim: 
         #rode com 20milhoes de elementos em máquina com 8GB assim
         ./roda-v2.sh 20000000 
         OU
         #rode com 500mil de elementos em máquina com 8GB assim
         ./roda-v2.sh 500000 
         
     esse script vai produzir a saida das experiencias para
     1 a 8 threads (10 vezes cada)
     Voce deve copiar (contol-C) a saida do script 
       na página dadosV2 na primeira celula dessa pagina
       fazer (control-V nessa célula)
       MARCANDO PARA A PLANILHA USAR COMO SEPARADORES OS CARACTERES []

     Ao fazer isso as tabelas estarão ok, e as cẽlulas também
     certifique-se que está tudo certo
     
     Aí basta voce ajustar as escalas dos gráficos 
     - clicando com mouse os eixos y (esquerdo ou direito) para
     e adequar as escalas, o prof. vai mostra na sala
     

Objetivos: 
// Obter uma implementação paralela MAIS eficiente para 
//   esse algoritmo "Soma de Prefixos" V1
//   em CPUs multicore usando PThreads

// será feita a soma de prefixos de 
//  um vetor de long (ou seja, INTEIROS DE 64BITS) 
//  usando nThreads (definida via linha de comando)

// Para ESSE lab faremos:
//   a soma de prefixos será com a operação SOMA  (+)

// Funcionamento:
//  O programa deve funcionar para nTotalElements com nThreads 
//   Com: nTotalElements e nThreads obtidos da linha de comando
//   (assim como a versão feita em aula para a redução)
//  Utilização do programa:
//  usage: ./prefixSumPth <nTotalElements> <nThreads>

// ENTRADA para o algoritmo:
// A entrada para a função PthPrefixSum será: 
//   um vetor (GLOBAL) de nTotalElements números inteiros
//   chamado InputVector
//   (nTotalElements obtido da linha de comando)
//
// Para esse teste o vetor NÂO será lido, 
//   - o vetor será preenchido sequencialmente pela função main
// Assim como nossa implementação anteriror, 
//   a inicializaçao do vetor de entrada (em main) deve ser
//
//      // initialize InputVector
        for( long i = 0; i < nTotalElements; i++ ){
	        int r = rand();  // Returns a pseudo-random integer
	                     //    between 0 and RAND_MAX.
		InputVector[i] = (r % 1000) - 500;
	}

// SAIDA do o algoritmo:
// A saída da função PthPrefixSum será: 
//   um vetor (GLOBAL) de nTotalElements números inteiros
//   chamado OutputVector
//   (nTotalElements obtido da linha de comando)
//

// o programa deve calcular e imprimir 
//   o tempo e a vazão de calculo da Soma de Prefixos usando
//   a VERSÂO 1 do algoritmo usando as idéias descritas
//   no arquivo: 
//     ideia-do-algoritmo-paralelo-SomaDePrefixos-Pthreads-V1.txt


// Verificaçao de correção do programa: 
// ------------------------------------
// (item incluido na versao 1)
// o programa main, antes de terminar, deve verificar
//    se seu algoritmo paralelo gerando corretamente o vetor de saída
//    essa verificação DEVE ser feita de forma sequencial 
//    (ao final do main), INCLUINDO-SE O CÓDIGO ABAIXO
//    esse código roda apenas ao final, e NÃO deve influenciar
//    na medida de tempo feita para o algoritmo paralelo e
//    nem deve influenciar no cálculo e resultado de VAZAO reportada

void verifyPrefixSum( const TYPE *InputVec, 
                      const TYPE *OutputVec, 
                      long nTotalElmts )
{
    volatile TYPE last = InputVec[0];
    int ok = 1;
    for( long i=1; i<nTotalElmts ; i++ ) {
           if( OutputVec[i] != (InputVec[i] + last) ) {
               fprintf( stderr, "In[%ld]= %ld\n"
                                "Out[%ld]= %ld (wrong result!)\n"
                                "Out[%ld]= %ld (ok)\n"
                                "last=%ld\n" , 
                                     i, InputVec[i],
                                     i, OutputVec[i],
                                     i-1, OutputVec[i-1],
                                     last );
               ok = 0;
               break;
           }
           last = OutputVec[i];    
    }  
    if( ok )
       printf( "\nPrefix Sum verified correctly.\n" );
    else
       printf( "\nPrefix Sum DID NOT compute correctly!!!\n" );   
}


// O código ACIMA DEVE SER chamado ao final de main
   da seguinte maneira para verificar que seus algoritmos funcionam:
   
   verifyPrefixSum( InputVector, OutputVector, nTotalElements );


// rodar o programa 10 vezes obtendo o tempo MÍNIMO e o MÉDIO
//  colocar TODOS os resultados em planilha

// Colocar em uma planilha (tabela) os tempos para
// 1, 2, 3, 4, 5, 6, 7, e 8 threads
// A última coluna da tabela (planilha) deve calcular 
//  a aceleração para 1, 2, 3, 4, 5, 6, 7, e 8 threads
// colocando em uma planilha (tabela)

// Entregar:
//  um tar.gz contendo:
//  -  o fonte da sua implementação em C, 
//  -  os scripts para compilar e rodar as experiências
//  -  a planilha preenchida com dados conforme descrito acima
//  -  Um arquivo com seu relatório descrevendo
//
//       a) como você implementou (descrever seu algoritmo)
//
//       b) a descrição do processador que voce usou, 
//          seu modelo e caracteristicas importantes para o experimento
//          COLOQUE EM APENDICE NO RELATORIO A SAIDA DO COMANDO lscpu

//          de preferência adicione também uma figura da 
//          topologia dos cores do processador obtida pelo programa lstopo

//       c) a descrição dos experimentos e como foram feitas as medidas
//       
//       d) a planilha de resultados sumarizando a vazao e aceleração
//       e) um gráfico (obtido de sua planilha) mostrando:
//            - no eixo X: o número de threads
//            - no eixo Y da esquerda: a vazao para cada número de threads
//                unir os pontos com linhas de uma certa cor C1 de sua preferência
//            - no eixo Y da direita: a aceleração para cada número de threads
//                unir os pontos com linhas de uma OUTRA cor C2 de sua preferência 

ENTREGA:
Em princípio o tar.gz deve ser entregue via upload na ufpr virtual
O professor deverá incluir (em breve) a opção para entrega do lab2 na ufpr virtual

--------------------------------------

