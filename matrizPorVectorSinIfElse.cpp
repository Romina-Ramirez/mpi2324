#include <iostream>
#include <mpi.h>
#include <vector>
#include <math.h>

#define M 25

void matrix_mult(double* A, double* b, double* c, int rows, int cols) {
    for (int i = 0; i < rows; i++){
        double tmp = 0;
        for(int j = 0; j < cols; j++){
            tmp = tmp + A[i*cols+j]*b[j];
        }
        c[i] = tmp;
    }
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int rows_per_rank;
    int rows_alloc = M;
    int padding = 0;

    if (M % nprocs != 0) {
        rows_alloc = std::ceil((double) M / nprocs) * nprocs;
        padding = rows_alloc - M;
    }

    rows_per_rank = rows_alloc / nprocs;

    std::vector<double> A(M * rows_alloc);
    std::vector<double> b(M);
    std::vector<double> c(rows_alloc);

    std::vector<double> A_local(M * rows_per_rank);
    std::vector<double> c_local(rows_per_rank);


    if (rank == 0){
        std::printf("Dimension: %d, rows_alloc: %d, rows_per_rank: %d, padding: %d \n",
                    M, rows_alloc, rows_per_rank, padding);

        for(int i = 0; i < M; i++){
            for(int j = 0; j < M; j++){
                int index = i * M + j;
                A[index] = i;
            }
        }

        for(int i = 0; i < M; i++) b[i] = 1;

    }

    MPI_Scatter(A.data(), M * rows_per_rank, MPI_DOUBLE,
                A_local.data(), M * rows_per_rank, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    std::printf("Rank_%d: [%.0f .. %.0f] \n", rank, A_local[0], A_local.back());

    MPI_Bcast(b.data(), M, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int rows_per_rank_tmp = rows_per_rank;
    if (rank == nprocs - 1){
        rows_per_rank_tmp = rows_per_rank - padding;
    }

    matrix_mult(A_local.data(), b.data(), c_local.data(), rows_per_rank_tmp, M);

    MPI_Gather(c_local.data(), rows_per_rank, MPI_DOUBLE,
               c.data(), rows_per_rank, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    c.resize(M);

    if (rank == 0){
        std::printf("Resultado: \n");

        for(int i = 0; i < M; i++){
            std::printf("%0.f, ", c[i]);
        }

        std::printf("\n");
    }

    MPI_Finalize();

    return 0;

}