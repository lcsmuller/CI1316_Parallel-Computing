#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <mpi.h>

#define NLA_DEFAULT 1000
#define M_DEFAULT 1000
#define NCB_DEFAULT 1000

double *create_matrix(int rows, int cols) {
    return malloc(rows * cols * sizeof(double));
}

void initialize_matrix(double *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = rand() / (double)RAND_MAX;
        }
    }
}

void print_process_info(int rank) {
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Get_processor_name(hostname, &len);
    printf("Process rank: %d, Host: %s\n", rank, hostname);
}

void print_matrix(double *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%lf ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

void matrix_multiply(double *A, double *B, double *C, int Nla, int M, int Ncb, int rank, int size) {
    const int rows_per_process = Nla / size, rows_remaining = Nla % size;
    const int start_row = rank * rows_per_process;
    int end_row = start_row + rows_per_process;

    if (rank == size - 1) {
        end_row += rows_remaining;
    }

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < Ncb; j++) {
            C[i * Ncb + j] = 0.0;
            for (int k = 0; k < M; k++) {
                C[i * Ncb + j] += A[i * M + k] * B[k * Ncb + j];
            }
        }
    }
}

void sequential_matrix_multiply(double *A, double *B, double *C, int Nla, int M, int Ncb) {
    for (int i = 0; i < Nla; i++) {
        for (int j = 0; j < Ncb; j++) {
            C[i * Ncb + j] = 0.0;
            for (int k = 0; k < M; k++) {
                C[i * Ncb + j] += A[i * M + k] * B[k * Ncb + j];
            }
        }
    }
}

int main(int argc, char **argv) {
    int rank, size;
    int Nla = NLA_DEFAULT;
    int M = M_DEFAULT;
    int Ncb = NCB_DEFAULT;
    int verify = 0;

    if (argc < 4) {
        printf("Usage: mpirun -np n ./mmul Nla M Ncb [-v]\n");
        return 1;
    }

    Nla = atoi(argv[1]);
    M = atoi(argv[2]);
    Ncb = atoi(argv[3]);

    if (argc == 5 && strcmp(argv[4], "-v") == 0) {
        verify = 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("Running parallel matrix multiplication using %d processes.\n", size);
        printf("Matrix A: [%d x %d]\n", Nla, M);
        printf("Matrix B: [%d x %d]\n", M, Ncb);
        printf("Matrix C: [%d x %d]\n", Nla, Ncb);
    }

    double *A, *B, *C;
    if (rank == 0) {
        A = create_matrix(Nla, M);
        B = create_matrix(M, Ncb);
        initialize_matrix(A, Nla, M);
        initialize_matrix(B, M, Ncb);
    }

    double *A_local = create_matrix(Nla, M), *C_local = create_matrix(Nla, Ncb);

    MPI_Scatter(A, Nla * M / size, MPI_DOUBLE, A_local, Nla * M / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, M * Ncb, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();
    matrix_multiply(A_local, B, C_local, Nla, M, Ncb, rank, size);
    double end_time = MPI_Wtime();

    MPI_Gather(C_local, Nla * Ncb / size, MPI_DOUBLE, C, Nla * Ncb / size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Parallel matrix multiplication completed.\n");
        printf("Time taken: %lf seconds.\n", end_time - start_time);
        double flops = (2.0 * Nla * Ncb * M) / (end_time - start_time) * 1e-9;
        printf("Throughput: %lf GFLOPS.\n", flops);

        if (verify) {
            double *C_seq = create_matrix(Nla, Ncb);
            start_time = MPI_Wtime();
            sequential_matrix_multiply(A, B, C_seq, Nla, M, Ncb);
            end_time = MPI_Wtime();
            printf("Running sequential matrix multiplication for verification.\n");
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
                if (!correct)
                    break;
            }

            printf("Parallel matrix multiplication result is %s.\n", correct ? "correct" : "incorrect");

            free(C_seq);
        }
    }

    if (rank == 0) {
        free(A);
        free(B);
        C = create_matrix(Nla, Ncb);
        print_matrix(C, Nla, Ncb);
        free(C);
    }

    free(A_local);
    free(B_local);
    free(C_local);

    MPI_Finalize();
    return 0;
}
