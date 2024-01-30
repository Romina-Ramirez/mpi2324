#include <iostream>
#include <mpi.h>
#include <vector>
#include <math.h>
#include <fstream>
#include <string>

std::vector<int> read_file() {
    std::fstream fs("datos.txt", std::ios::in);
    std::string line;

    std::vector<int> ret;

    while (std::getline(fs, line)) {
        ret.push_back(std::stoi(line));
    }

    fs.close();

    return ret;
}

int sumar(int *tmp, int n) {
    int suma = 0;
    for (int i = 0; i < n; i++) {
        suma += tmp[i];
        std::printf("%d", suma);
    }
    return suma;
}

void tabla(std::vector<int> frec) {
    std::printf("+-------+--------+ \n");
    std::printf("| Valor | Conteo | \n");
    std::printf("+-------+--------+ \n");

    for (int i = 0; i <= 100; i++) {
        std::printf("|  %3d  | %5d | \n", i, frec[i]);
    }

    std::printf("+-------+--------+ \n");
}

void frecuencias(std::vector<int> datos, int n, int nprocs, int rank) {
    
    int block_size = floor(n / nprocs);
    int block_size0 = n - (block_size * (nprocs - 1));
    int dif = block_size0 - block_size;

    if (rank == 0){
        std::vector<int> frecuenciasTotal(101);

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            int start = (rank_id * block_size) + dif;
            MPI_Send(&datos[start], block_size, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
        }

        for (int i = 0; i < block_size0; i++) {
            frecuenciasTotal[datos[i]]++;
        }

        MPI_Reduce(MPI_IN_PLACE, &frecuenciasTotal, 101, 
                    MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        tabla(frecuenciasTotal);

    } else {
        std::vector<int> frecuencias(101);
        std::vector<int> datos_local(block_size);

        MPI_Recv(&datos_local, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < block_size; i++) {
            frecuencias[datos_local[i]]++;
        }

        MPI_Reduce(&frecuencias, MPI_IN_PLACE, 101, 
                    MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }

}

void promedio(std::vector<int> datos, int n, int nprocs, int rank) {
    
    int block_size = floor(n / nprocs);
    int block_size0 = n - (block_size * (nprocs - 1));
    int dif = block_size0 - block_size;

    if (rank == 0){
        double promedio = 0;

        std::printf("Dimension: %d, block_size: %d, block_size0: %d, dif: %d \n",
                    n, block_size, block_size0, dif);

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            int start = (rank_id * block_size) + dif;
            MPI_Send(&datos[start], block_size, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
        }

        int suma = sumar(datos.data(), block_size0);

        MPI_Reduce(MPI_IN_PLACE, &suma, 1, 
                    MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        promedio = suma / n;

        std::printf("Promedio: %f \n", promedio);

    } else {
        int suma_parcial;
        std::vector<int> datos_local(block_size);

        MPI_Recv(&datos_local, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        suma_parcial = sumar(datos_local.data(), block_size);

        MPI_Reduce(&suma_parcial, MPI_IN_PLACE, 1, 
                    MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }

}

void min_max(std::vector<int> datos, int n, int nprocs, int rank) {
    
    int min;
    int max;

    int block_size = floor(n / nprocs);
    int block_size0 = n - (block_size * (nprocs - 1));
    int dif = block_size0 - block_size;

    if (rank == 0){
        int min = datos[0];
        int max = datos[0];

        std::printf("Dimension: %d, block_size: %d, block_size0: %d, dif: %d \n",
                    n, block_size, block_size0, dif);

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            int start = (rank_id * block_size) + dif;
            MPI_Send(&datos[start], block_size, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
        }

        for (int i = 0; i < block_size0; i++) {
            if (datos[i] < min)
                min = datos[i];
            if (datos[i] > max)
                max = datos[i];
        }

        MPI_Reduce(MPI_IN_PLACE, &min, 1, 
                    MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

        MPI_Reduce(MPI_IN_PLACE, &max, 1, 
                    MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);            

        std::printf("MIN: %d, MAX: %d \n", min, max);

    } else {
        std::vector<int> datos_local(block_size);

        MPI_Recv(&datos_local, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int min_local = datos_local[0];
        int max_local = datos_local[0];

        for (int i = 0; i < block_size; i++) {
            if (datos[i] < min_local)
                min_local = datos[i];
            if (datos[i] > max_local)
                max_local = datos[i];
        }

        MPI_Reduce(&min_local, MPI_IN_PLACE, 1, 
                    MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

        MPI_Reduce(&max_local, MPI_IN_PLACE, 1, 
                    MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD); 
    }

}


int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> datos;

    if(rank == 0){
        datos = read_file();
    }

    //frecuencias(datos, datos.size(), nprocs, rank);
    //promedio(datos, datos.size(), nprocs, rank);
    min_max(datos, datos.size(), nprocs, rank);
    
    MPI_Finalize();

    return 0;

}