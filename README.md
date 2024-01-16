# mpi2324

## Instalación de MPI


sudo apt-get update
sudo apt-get install libopenmpi-dev


## Compilación

mpicxx ejemplo.cpp -o ejemplo


## Ejecutar

mpiexec -n procs ./ejemplo