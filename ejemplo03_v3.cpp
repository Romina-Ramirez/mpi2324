#include <iostream>
#include <mpi.h>
#include <math.h>
#include <vector>

#define N 25

int sumar(int *tmp, int n) {
    int suma = 0;
    for (int i = 0; i < n; i++) {
        suma += tmp[i];
    }
    return suma;
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size;
    int real_size;
    int padding;

    if(N%nprocs != 0){
        real_size = std::ceil((double) N / nprocs) * nprocs;
        block_size = real_size / nprocs;
        padding = real_size - N;
    }

    std::vector<int> data;
    std::vector<int> data_local(block_size);

    if (rank == 0){
        // Inicializar
        data.resize(real_size);

        std::printf("Dimension: %d, real_size: %d, block_size: %d, padding: %d \n",
                    N, real_size, block_size, padding);

        for (int i = 0; i < N; i++){
            data [i] = i;
        }
    }

    // Enviar los datos a cada rank
    MPI_Scatter(data.data(), block_size, MPI_INT, 
                data_local.data(), block_size, MPI_INT, 
                0, MPI_COMM_WORLD);

    // Calculo suma parcial
    if (rank == nprocs - 1){
         block_size = block_size - padding;
    }

    int suma_parcial = sumar(data_local.data(), block_size);

    std::printf("RANK_%d: suma parcial = %d \n", rank, suma_parcial);

    // Enviar las sumas parciales al rank 0
    int suma_total = 0;
    MPI_Reduce(&suma_parcial, &suma_total, 1, MPI_INT, 
                MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0){
        std::printf("Suma total: %d \n", suma_total);
    }

    MPI_Finalize();

    return 0;

}