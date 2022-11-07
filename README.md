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
### How to run
```bash
make parallelEncoder
bin/parallelEncode.out data/test.bin data/test.bin.huf
make parallelDecoder
bin/parallelDecode.out data/test.bin.huf data/test.bin.huf.dec
```
### Test against valgrind
```bash
gcc -g src/serial/encode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c
valgrind -s --leak-check=full --show-leak-kinds=all ./a.out ./data/test.bin
```


