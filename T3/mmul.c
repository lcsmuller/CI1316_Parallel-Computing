#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <openmpi/mpi.h>

#define NLA_DEFAULT 100
#define M_DEFAULT   100
#define NCB_DEFAULT 100

void
initialize_matrix(double *array, int size)
{
    for (int i = 0; i < size; i++) {
        array[i] = rand() / (double)RAND_MAX;
    }
}

void
print_process_info(int rank)
{
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Get_processor_name(hostname, &len);
    printf("Process rank: %d, Host: %s\n", rank, hostname);
}

void
print_matrix(double *array, int rows, int cols)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%lf ", array[i * cols + j]);
        }
        printf("\n");
    }
}

void
matrix_multiply(
    double *A, double *B, double *C, int start, int end, int M, int Ncb)
{
    for (int i = start; i < end; i++) {
        for (int j = 0; j < Ncb; j++) {
            C[i * Ncb + j] = 0.0;
            for (int k = 0; k < M; k++) {
                C[i * Ncb + j] += A[i * M + k] * B[k * Ncb + j];
            }
        }
    }
}

#define CHECK_MULTIPLE(value) (!((value) % 3) && !((value) % 4) && !((value) % 2))

int
main(int argc, char **argv)
{
    int rank, size;
    int Nla = NLA_DEFAULT;
    int M = M_DEFAULT;
    int Ncb = NCB_DEFAULT;
    int verify = 0;

    if (argc < 4) {
        fprintf(stdout, "Usage: mpirun -np n ./mmul Nla M Ncb [-v]\n");
        return EXIT_FAILURE;
    }

    Nla = atoi(argv[1]);
    M = atoi(argv[2]);
    Ncb = atoi(argv[3]);

    if (!(CHECK_MULTIPLE(Nla) && CHECK_MULTIPLE(M) && CHECK_MULTIPLE(Ncb))) {
        fprintf(stderr, "Nla, M and Ncb must be a multiple of 2, 3 and 4\n"
                        "Nla:\t%d\nM:\t%d\nNcb:\t%d", Nla, M, Ncb);
        return EXIT_FAILURE;
    }

    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
        verify = 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Running parallel matrix multiplication using %d processes.\n",
               size);
        printf("Matrix A: [%d x %d]\n", Nla, M);
        printf("Matrix B: [%d x %d]\n", M, Ncb);
        printf("Matrix C: [%d x %d]\n", Nla, Ncb);
    }

    double *A = calloc(Nla * M, sizeof(double)),
           *B = calloc(M * Ncb, sizeof(double)),
           *C = calloc(Nla * Ncb, sizeof(double));

    if (rank == 0) {
        initialize_matrix(A, Nla * M);
        initialize_matrix(B, M * Ncb);
    }

    // broadcast B matrix
    MPI_Bcast(B, M * Ncb, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // scatter A matrix
    int sendcounts = (Nla / size) * M;
    double *local_A = calloc(sendcounts, sizeof(double)),
           *local_C = calloc(Nla * Ncb / size, sizeof(double));

    double start_time, end_time;

    if (rank == 0) start_time = MPI_Wtime();

    // scatter A matrix
    MPI_Scatter(A, sendcounts, MPI_DOUBLE, local_A, sendcounts, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // perform matrix multiplication on each processor
    matrix_multiply(local_A, B, local_C, 0, sendcounts / M, M, Ncb);
    // gather C matrix
    sendcounts = (Nla * Ncb) / size;
    MPI_Gather(local_C, sendcounts, MPI_DOUBLE, C, sendcounts,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Parallel matrix multiplication completed.\n");
        printf("Time taken: %lf seconds.\n", end_time - start_time);
        double flops = (2.0 * Nla * Ncb * M) / (end_time - start_time) * 1e-9;
        printf("Throughput: %lf GFLOPS.\n", flops);

        if (verify) {
            double *C_seq = calloc(Nla * Ncb, sizeof(double));
            start_time = MPI_Wtime();
            matrix_multiply(A, B, C_seq, 0, Nla, M, Ncb);
            end_time = MPI_Wtime();
            printf("Running sequential matrix multiplication for "
                   "verification.\n");
            printf("Time taken: %lf seconds.\n", end_time - start_time);

            int correct = 1;
            const double epsilon = 1e-6;
            for (int i = 0; i < Nla; i++) {
                for (int j = 0; j < Ncb; j++) {
                    if (fabs(C[i * Ncb + j] - C_seq[i * Ncb + j]) > epsilon) {
                        correct = 0;
                        break;
                    }
                }
                if (!correct) break;
            }

            printf("Parallel matrix multiplication result is %s.\n",
                   correct ? "correct" : "incorrect");

            free(C_seq);
        }
    }

    free(local_A);
    free(local_C);
    free(A);
    free(B);
    free(C);

    MPI_Finalize();
    return 0;
}
