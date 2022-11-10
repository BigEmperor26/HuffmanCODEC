# Parallel Huffman Coding and Encoding

This repository contains the High Performance Computing for Data Science project for:
- Bozzo Francesco, 229312
- Yin Michele, 229359

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
make build
```
### How to encode
```bash
bin/main.out -e data/test.bin data/test.bin.huf
```
### How to decode
```bash
bin/main.out -d data/test.bin data/test.bin.huf
```
### Options
```bash
-e to encode
-d to decode
-p to set the number of threads to work on each file
-r process a folder
-h help text
```
### Test against valgrind
```bash
gcc -g src/serial/encode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c
valgrind -s --leak-check=full --show-leak-kinds=all ./a.out ./data/test.bin
```


