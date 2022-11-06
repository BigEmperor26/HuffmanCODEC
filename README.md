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

### Test against valgrind
```bash
gcc -g src/serial/encode.c src/datastructures/priorityQ.c src/datastructures/dictionary.c src/serial/huffman.c
valgrind -s --leak-check=full --show-leak-kinds=all ./a.out ./data/test.bin
```