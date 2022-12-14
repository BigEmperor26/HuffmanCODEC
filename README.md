# Parallel Huffman Coding and Decoding

This repository contains the High Performance Computing for Data Science project for:
- Bozzo Francesco, 229312
- Yin Michele, 229359

The code requires MPI and OPENMP for multiprocessing and multithreading respectively. If OPENMP is not present, it is possible to compile and run the code without it, running single threaded
## Architecture
<img src="./docs/imgs/overall%20flow%20schema.png" alt="drawing" width="500"/>

## Report
Link to the report with analysis of perfomances and discussion about design choices
[Report](./docs/report/main.pdf)
## Presentation
Link to the presentation discussion about design choices
[Report](./docs/presentation/main.pdf)
## Serial
### How to run

```bash
clear && gcc src/serial/encode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c && ./a.out data/test.bin
clear && gcc src/serial/decode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c && ./a.out data/test.bin.huf
```
alternatively 
```bash
# make files
make encoder
make decoder
# encode and decode respectively
bin/encode.out data/test.bin
bin/decode.out data/test.bin.huf
```
## Parallel
### Make
```bash
# MPI and OPENMP
make build
# ONLY MPI, no multithreading
make build_no_openmp
```
### How to encode
```bash
# single process, single threaded
bin/main.out -e data/test.bin data/test.bin.huf
# multithread
export OMP_NUM_THREADS=threads
bin/main.out -e data/test.bin data/test.bin.huf
# multiprocess folders
mpiexec -np processes bin/main.out -e -r data/ data_huf/
# multiprocess folders and multithread files
export OMP_NUM_THREADS=threads
mpiexec -np processes bin/main.out -e -r data/ data_huf/
```
### How to decode
```bash
# single process, single threaded
bin/main.out -d data/test.bin.huf data/test.bin.dec
# multithread
export OMP_NUM_THREADS=threads
bin/main.out -d data/test.bin.huf data/test.bin.dec
# multiprocess folders
mpiexec -np processes bin/main.out -d -r data_huf/ data_dec/
# multiprocess folders and multithread files
export OMP_NUM_THREADS=threads
mpiexec -np processes bin/main.out -d -r data_huf/ data_dec/
```
### Options
```bash
-e to encode
-d to decode
-r process a folder
-b blocking synchronization ( default selection )
-l locks synchronization
-h help text
```
### Test against valgrind

Note, that the libomp.h library has a memory leak while creating multiple threads, so if this command is run on the cluster, there will always be 1 error detected by valgrind, unless only 1 thread is used

```bash
## serial
make encoder
valgrind --leak-check=full --show-leak-kinds=all ./bin/encode.out ./data/test.bin
## serial
make decoder
valgrind --leak-check=full --show-leak-kinds=all ./bin/decode.out ./data/test.bin.huf
## parallel
make build
export OMP_NUM_THREADS=threads
valgrind --leak-check=full --show-leak-kinds=all ./bin/main.out -e ./data/test.bin ./data/test.bin.huf
mpiexec -n 4 valgrind --leak-check=full --show-leak-kinds=all ./bin/main.out -e -r ./data ./data_huf
## parallel
make build
export OMP_NUM_THREADS=threads
valgrind --leak-check=full --show-leak-kinds=all ./bin/main.out -d ./data/test.bin.huf ./data/test.bin.dec
mpiexec -n 4 valgrind --leak-check=full --show-leak-kinds=all ./bin/main.out -d -r ./data_huf ./data_dec
```


