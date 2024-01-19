#include <iostream>
#include <mpi.h>
#include<math.h>

#define N 9

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
    int data[N];

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size = floor(N / nprocs);
    int block_size0 = N - (block_size * (nprocs - 1));
    int dif = block_size0 - block_size;

    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            data[i] = i;
        }

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            int start = (rank_id * block_size) + dif;
            MPI_Send(&data[start], block_size, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
        }

        int suma_ranks[nprocs];

        suma_ranks[0] = sumar(data, block_size0);

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            MPI_Recv(&suma_ranks[rank_id], 1, MPI_INT, rank_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int suma_total = sumar(suma_ranks, nprocs);
        std::printf("Suma total: %d \n", suma_total);
    } else {
        MPI_Recv(data, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::string str = " ";
        for (int i = 0; i < block_size; i++) {
            str = str + std::to_string(data[i]) + ", ";
        }
        std::printf("RANK_%d datos recibidos ==> %s \n", rank, str.c_str());

        int suma_parcial = sumar(data, block_size);
        std::printf("RANK_%d suma ==> %d \n", rank, suma_parcial);
        MPI_Send(&suma_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;

}