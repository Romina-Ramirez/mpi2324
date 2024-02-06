# mpi2324

## Instalación de MPI


sudo apt-get update
sudo apt-get install libopenmpi-dev


## Compilación

mpicxx ejemplo.cpp -o ejemplo
mpicxx correccion.cpp -o correccion


## Ejecutar

mpiexec -n procs ./ejemplo
mpiexec -n 4 ./correccion