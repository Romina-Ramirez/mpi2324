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
    
    if(N % nprocs != 0){
        real_size = std::ceil((double) N / nprocs) * nprocs;
        block_size = real_size / nprocs;
        padding = real_size - N;
    }

    if (rank == 0){
        int suma_total = 0;
        std::vector<int> data(real_size);

        std::printf("Dimension: %d, real_size: %d, block_size: %d, padding: %d \n",
                    N, real_size, block_size, padding);

        for (int i = 0; i < N; i++){
            data [i] = i;
        }

        MPI_Scatter(data.data(), block_size, MPI_INT, 
                    MPI_IN_PLACE, 0, MPI_INT, 
                    0, MPI_COMM_WORLD);

        suma_total += sumar(data.data(), block_size);

        std::printf("RANK_%d: suma parcial = %d \n", rank, suma_total);

        MPI_Reduce(MPI_IN_PLACE, &suma_total, 1, MPI_INT, 
                MPI_SUM, 0, MPI_COMM_WORLD);

        std::printf("Suma total: %d \n", suma_total);

    } else {

        int suma_parcial;
        std::vector<int> data_local(block_size);

        MPI_Scatter(nullptr, 0, MPI_INT, 
                    data_local.data(), block_size, MPI_INT, 
                    0, MPI_COMM_WORLD);
        
        if (rank == nprocs - 1){
            block_size = block_size - padding;
        }

        suma_parcial = sumar(data_local.data(), block_size);

        std::printf("RANK_%d: suma parcial = %d \n", rank, suma_parcial);

        MPI_Reduce(&suma_parcial, MPI_IN_PLACE, 1, MPI_INT, 
                MPI_SUM, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;

}