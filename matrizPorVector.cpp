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

    if (rank == 0){
        // Imprimir informacion
        std::printf("Dimension: %d, rows_alloc: %d, rows_per_rank: %d, padding: %d \n",
                    M, rows_alloc, rows_per_rank, padding);

    // Inicializar las variables a utilizar

        // El número de filas se completa con el padding y las columnas con el tamaño predefinido
        std::vector<double> A(M * rows_alloc);

        // El número de filas se mantiene con el tamaño predefinido
        std::vector<double> b(M);

        // El número de filas se completa con el padding
        std::vector<double> c(rows_alloc);

        for(int i = 0; i < M; i++){
            for(int j = 0; j < M; j++){
                int index = i * M + j;
                A[index] = i;
            }
        } 

        for(int i = 0; i < M; i++) b[i] = 1;

    // Enviar la matriz A mediante el Scatter 
        MPI_Scatter(A.data(), M * rows_per_rank, MPI_DOUBLE, 
                    MPI_IN_PLACE, 0, MPI_DOUBLE, 
                    0, MPI_COMM_WORLD);

    // Enviar el vector b mediante Bcast
        MPI_Bcast(b.data(), M, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Realizar el calculo c = A x b
        matrix_mult(A.data(), b.data(), c.data(), rows_per_rank, M);

    // Recibir el vector c
        MPI_Gather(MPI_IN_PLACE, 0, MPI_DOUBLE, 
                    c.data(), rows_per_rank, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);
        
        c.resize(M);

    // Imprimir el resultado
        std::printf("Resultado: \n");

        for(int i = 0; i < M; i++){
            std::printf("%0.f, ", c[i]);
        } 

        std::printf("\n");

    } else {
        std::vector<double> A_local(M * rows_per_rank);
        std::vector<double> b_local(M);
        std::vector<double> c_local(rows_per_rank);

    // Recibir la matriz A
        MPI_Scatter(nullptr, 0, MPI_DOUBLE, 
                    A_local.data(), M * rows_per_rank, MPI_DOUBLE, 
                    0, MPI_COMM_WORLD);

        std::printf("Rank_%d: [%.0f .. %.0f] \n", rank, A_local[0], A_local.back());

    // Recibir el vector b
        MPI_Bcast(b_local.data(), M, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Realizar el calculo c = A x b
        int rows_per_rank_tmp = rows_per_rank;
        if (rank == nprocs - 1){
            rows_per_rank_tmp = rows_per_rank - padding;
            //rows_per_rank_tmp = M - rank * rows_per_rank;
        }

        matrix_mult(A_local.data(), b_local.data(), c_local.data(), rows_per_rank_tmp, M);

    // Enviar el vector c
        MPI_Gather(c_local.data(), rows_per_rank, MPI_DOUBLE, 
                    nullptr, 0, MPI_DOUBLE,
                    0, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    return 0;

}