# Parallel Huffman Coding and Encoding

This repository contains the High Performance Computing for Data Science project for:
- Bozzo Francesco, 229312
- Yin Michele, 229359

The code requires MPI and OPENMP for multiprocessing and multithreading respectively. If OPENMP is not present, it is possible to compile and run the code without it, running single threaded

## Serial
### How to run

```bash
clear && gcc src/serial/encode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c && ./a.out data/test.bin
clear && gcc src/serial/decode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c && ./a.out data/test.bin.huf
```

```bash
make encoder
bin/encode.out data/test.bin
make decoder
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
mpiexec -np processes bin/main.out -e -r data/ data_enc/
# multiprocess folders and multithread files
export OMP_NUM_THREADS=threads
mpiexec -np processes bin/main.out -e -r data/ data_enc/
```
### How to decode
```bash
# single process, single threaded
bin/main.out -d data/test.bin.huf data/test.bin.dec
# multithread
export OMP_NUM_THREADS=threads
bin/main.out -d data/test.bin.huf data/test.bin.dec
# multiprocess folders
mpiexec -np processes bin/main.out -d -r data_enc/ data_dec/
# multiprocess folders and multithread files
export OMP_NUM_THREADS=threads
mpiexec -np processes bin/main.out -d -r data_enc/ data_dec/
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
```bash
gcc -g src/serial/encode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c
valgrind -s --leak-check=full --show-leak-kinds=all ./a.out ./data/test.bin
```


