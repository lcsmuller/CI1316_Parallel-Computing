// TRABALHO2: CI316 1o semestre 2023 (myBroadcast_rb.c)
// Aluno: Lucas Müller
// GRR: 20197160
// Aluno: Tiago Serique
// GRR: 20195138
//

// Esqueleto de programa principal disponibilizado pelo prof!

//////////////////////////////////////////////
///// ATENCAO: NAO MUDAR O MAIN, a       /////
/////  menos que seja MESMO necessário!  /////
//////////////////////////////////////////////

#include <openmpi/mpi.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "chrono.c"

#define USE_MPI_Bcast 1 // do NOT change
#define USE_my_Bcast  2 // do NOT change
// choose either BCAST_TYPE in the defines bellow
//#define BCAST_TYPE USE_MPI_Bcast
#define BCAST_TYPE USE_my_Bcast

long nmsg; // o número total de mensagens
long tmsg; // o tamanho de cada mensagem
int nproc; // o número de processos MPI
int raiz; // maquina que ira enviar as mensagens
int processId; // rank dos processos
int ni; // tamanho do vetor contendo as mensagens

chronometer_t myBroadcastChrono;

//#define DEBUG 1
#define DEBUG 0

#if DEBUG == 1
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

const int SEED = 100;

// MACROS para AJUDAR!
#define LOGIC_RANK(my_rank, root, comm_size)                                  \
    ((my_rank + comm_size - root) % comm_size)
#define PHYSIC_RANK(logic_rank, root, comm_size)                              \
    ((logic_rank + root) % comm_size)
#define LOGIC_SOURCE(logic_rank) (~(1 << (int)log2(logic_rank)) & (logic_rank))

void
my_Bcast_rb(
    void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    void *rbuffer = &((long *)buffer)[count / 2];
    const char *nodeType = "root";
    int rankFisico, nNodos;

    MPI_Comm_rank(comm, &rankFisico);
    MPI_Comm_size(comm, &nNodos);

    if (nNodos == 1) return;

    const int rankLogico = LOGIC_RANK(rankFisico, root, nNodos);

    // Se não for root, espera receber sua metade do buffer
    if (rankFisico != root) {
        const int src = PHYSIC_RANK(LOGIC_SOURCE(rankLogico), root, nNodos);
        if (rankLogico % 2 == 0)
            MPI_Recv(buffer, count / 2, datatype, src, 0, comm,
                     MPI_STATUS_IGNORE);
        else
            MPI_Recv(rbuffer, count / 2 + count % 2, datatype, src, 0, comm,
                     MPI_STATUS_IGNORE);

        nodeType = "node";
    }

    // Realiza envios de sua metade do buffer
    for (int fase = 0, end = ceil((double)log2(nNodos)); fase < end; fase++) {
        const int np = pow(2, fase);

        if (rankLogico >= np || (rankLogico + np) >= nNodos) continue;

        const int destLogico = rankLogico + pow(2, fase),
                  destFisico = PHYSIC_RANK(destLogico, root, nNodos);
        DEBUG_PRINT("(%s) rank: %d\tdest: %d\tfase: %d\tnNodos:"
                    "%d\n",
                    nodeType, rankFisico, destFisico, fase + 1, nNodos);
        if (destLogico % 2 == 0)
            MPI_Send(buffer, count / 2, datatype, destFisico, 0, comm);
        else
            MPI_Send(rbuffer, count / 2 + count % 2, datatype, destFisico, 0,
                     comm);
    }

    // Completa outra metade do buffer a partir do nodo vizinho que contém a
    // outra metade
    const int ultimoNodoPar = nNodos - 1, ultimoNodoImpar = nNodos - 2;

    if (rankLogico % 2 == 0 && rankLogico != ultimoNodoPar) {
        const int destLogico = rankLogico + 1,
                  destFisico = PHYSIC_RANK(destLogico, root, nNodos);

        DEBUG_PRINT("Enviando de %d para %d\n", rankFisico, destFisico);
        MPI_Send(buffer, count / 2, datatype, destFisico, 0, comm);

        DEBUG_PRINT("Recebendo de %d para %d\n", rankFisico, destFisico);
        MPI_Recv(rbuffer, count / 2 + count % 2, datatype, destFisico, 0, comm,
                 MPI_STATUS_IGNORE);
    }
    else if (rankLogico % 2 == 1) {
        const int destLogico = rankLogico - 1,
                  destFisico = PHYSIC_RANK(destLogico, root, nNodos);

        DEBUG_PRINT("Recebendo de %d para %d\n", rankFisico, destFisico);
        MPI_Recv(buffer, count / 2, datatype, destFisico, 0, comm,
                 MPI_STATUS_IGNORE);

        DEBUG_PRINT("Enviando de %d para %d\n", rankFisico, destFisico);
        MPI_Send(rbuffer, count / 2 + count % 2, datatype, destFisico, 0,
                 comm);

        if (rankLogico == ultimoNodoImpar) {
            const int destLogico = rankLogico + 1,
                      destFisico = PHYSIC_RANK(destLogico, root, nNodos);

            DEBUG_PRINT("Enviando de %d para %d\n", rankFisico, destFisico);
            MPI_Send(rbuffer, count / 2 + count % 2, datatype, destFisico, 0,
                     comm);
        }
    }
    else {
        const int src = PHYSIC_RANK(rankLogico - 1, root, nNodos);

        DEBUG_PRINT("Recebendo de %d para %d\n", rankFisico, destFisico);
        MPI_Recv(rbuffer, count / 2 + count % 2, datatype, src, 0, comm,
                 MPI_STATUS_IGNORE);
    }
}

// OBS1: sua função my_Bcast_rb
// deve ter o mesmo protótipo da MPI_Bcast
// ou seja, os dois protótipos são:
//  int MPI_Bcast( void *buffer, int count, MPI_Datatype datatype, int root,
//  MPI_Comm comm ); void my_Bcast_rb( void *buffer, int count, MPI_Datatype
//  datatype, int root, MPI_Comm comm );
// PORÉM:
// a sua função só precisa funcionar se chamada com datatype MPI_LONG

// OBS2:
// O Prof. vai disponibilizar a função
// verifica_my_Bcast_rb( ... )
// Essa função deve ser chamada AO FINAL do seu programa,
// DEPOIS que ele apresentar suas medições,
// E antes de desalocar seu buffer, e também ANTES de finalizar o
// MPI. Essa função vai verificar se sua função my_Bcast está
// funcionando adequadamente
// O tempo gasto nessa função de verificação
//  NAO deve influenciar suas medidas.

void
verifica_my_Bcast_rb(
    void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    static long *buff;
    int comm_size;
    int my_rank;

    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &my_rank);
    buff = (long *)calloc(count * comm_size, sizeof(long));

    // preenche a faixa do raiz com alguma coisa (apenas no raiz)
    if (my_rank == root)
        for (int i = 0; i < count; i++)
            buff[i] = i + SEED;

#if BCAST_TYPE == USE_MPI_Bcast
    MPI_Bcast(buff, count, datatype, root, comm);
#elif BCAST_TYPE == USE_my_Bcast
    my_Bcast_rb(buff, count, datatype, root, comm);
#else
    assert(BCAST_TYPE == USE_MPI_Bcast || BCAST_TYPE == USE_my_Bcast);
#endif

    // cada nodo verifica se sua faixa recebeu adequadamente o conteudo
    int ok = 1;
    int i;
    for (i = 0; i < count; i++)
        if (buff[i] != i + SEED) {
            ok = 0;
            break;
        }
    // imprime na tela se OK ou nao
    if (ok)
        fprintf(stderr, "MY BCAST VERIF: node %d received ok\n", my_rank);
    else
        fprintf(stderr,
                "MY BCAST VERIF: node %d NOT ok! local position: %d contains "
                "%ld\n",
                my_rank, i, buff[i]);

    free(buff);
}

int
main(int argc, char *argv[])
{

    // wz_debug = 1;    // DEBUG_MPI_MESSAGES

    raiz = 0;

    if (argc < 4) {
        printf("usage: mpirun -np <np> %s <nmsg> <tmsg> <nproc> (-r <r>)\n",
               argv[0]);
        return 0;
    }
    else {
        nmsg = atoi(argv[1]);
        tmsg = atoi(argv[2]);
        if (tmsg % 8 != 0) {
            printf(
                "usage: mpirun -np <np> %s <nmsg> <tmsg> <nproc> (-r <r>)\n",
                argv[0]);
            printf("<tmsg> deve ser multiplo de 8\n");
            return 0;
        }
        nproc = atoi(argv[3]);
        if (argc == 6) {
            if (strcmp(argv[4], "-r") == 0) raiz = atoi(argv[5]);
        }
    }

    ni =
        tmsg / sizeof(long int); // quantidade de inteiros longos nas mensagens
    MPI_Status Stat;

    // aloca a mensagem
    long int *inmsg = (long int *)calloc(ni, sizeof(long int));

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    if (processId == 0) {
#if BCAST_TYPE == USE_MPI_Bcast
        fprintf(stderr, "\n****** USING MPI_Bcast ******\n");
#elif BCAST_TYPE == USE_my_Bcast
        fprintf(stderr, "\n****** USING my_Bcast_rb ******\n");
#else
        assert(BCAST_TYPE == USE_MPI_Bcast || BCAST_TYPE == USE_my_Bcast);
#endif
    }

    // check my macros
    if (processId == 0) {
        fprintf(stderr, "\n---------ranks-------\n");
        for (int phys = 0; phys < nproc; phys++)
            fprintf(stderr, "%d ", phys);
        fprintf(stderr, " PHYSIC\n");
        for (int phys = 0; phys < nproc; phys++)
            fprintf(stderr, "%d ", LOGIC_RANK(phys, raiz, nproc));
        fprintf(stderr, " LOGIC from PHYSIC\n");
        for (int phys = 0; phys < nproc; phys++) {
            int logic_no = LOGIC_RANK(phys, raiz, nproc);
            fprintf(stderr, "%d ", PHYSIC_RANK(logic_no, raiz, nproc));
        }
        fprintf(stderr, " PHYSIC from LOGIC\n");
        fprintf(stderr, "\n---------------------\n");
    }

    // preenche a mensagem
    if (processId == raiz) {
        for (long int i = 1; i <= ni; i++)
            inmsg[i - 1] = i + SEED;
    }

    if (processId == 0)
        printf("----- root= %d, nmessages=%ld, nlong=%d"
               " argc:%d argv[4]=%s argv[5]=%s comm_size=%d\n",
               raiz, nmsg, ni, argc, argv[4], argv[5], nproc);

    MPI_Barrier(MPI_COMM_WORLD);

    if (processId == 0) {
        chrono_reset(&myBroadcastChrono);
        chrono_start(&myBroadcastChrono);
    }

    for (int m = 0; m < nmsg; m++)
#if BCAST_TYPE == USE_MPI_Bcast
        MPI_Bcast_rb(inmsg, ni, MPI_LONG, raiz, MPI_COMM_WORLD);
#elif BCAST_TYPE == USE_my_Bcast
        my_Bcast_rb(inmsg, ni, MPI_LONG, raiz, MPI_COMM_WORLD);
#else
        assert(BCAST_TYPE == USE_MPI_Bcast || BCAST_TYPE == USE_my_Bcast);
#endif

    MPI_Barrier(MPI_COMM_WORLD);

    if (processId == 0) {
        chrono_stop(&myBroadcastChrono);
        chrono_reportTime(&myBroadcastChrono, "myBroadcastChrono");

        // calcular e imprimir a VAZAO (nesse caso: numero de BYTES/s)
        double total_time_in_seconds =
            (double)chrono_gettotal(&myBroadcastChrono)
            / ((double)1000 * 1000 * 1000);
        double total_time_in_micro =
            (double)chrono_gettotal(&myBroadcastChrono) / ((double)1000);
        printf("total_time_in_seconds: %lf s\n", total_time_in_seconds);
        printf("Latencia: %lf us (CADA broadcast)\n",
               (total_time_in_micro / nmsg));
        double MBPS = (((double)nmsg * tmsg)
                       / ((double)total_time_in_seconds * 1000 * 1000));
        printf("Throughput: %lf MB/s\n", MBPS * (nproc - 1));
    }

#if DEBUG == 1
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("rank %d: ", rank);
    for (long int i = 0; i < 2; i++)
        printf("%ld ", inmsg[i]);
    for (long int i = ni - 2; i < ni; i++)
        printf("%ld ", inmsg[i]);
    printf("\n");
#endif

    // wz_debug = 1;
    // verifica_my_Bcast a partir de raiz 0  ---------- COM valores da linha de
    // comando
    verifica_my_Bcast_rb(inmsg, ni, MPI_LONG, raiz, MPI_COMM_WORLD);

    // verifica_my_Bcast a partir de raiz 0 ---------- COM OUTROS valores
    // verifica_my_Bcast( inmsg, 7, MPI_LONG, 0, MPI_COMM_WORLD );
    // verifica_my_Bcast a partir de raiz 1
    // verifica_my_Bcast( inmsg, 7, MPI_LONG, 1, MPI_COMM_WORLD );
    // verifica_my_Bcast a partir de raiz 3
    // verifica_my_Bcast( inmsg, 7, MPI_LONG, 3, MPI_COMM_WORLD );

    free(inmsg);

    MPI_Finalize();
    return 0;
}
