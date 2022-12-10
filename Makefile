BIN_FOLDER=./bin/

encoder:
	mkdir -p $(BIN_FOLDER)
	gcc -std=c99 -o ./bin/encode.out ./src/serial/encode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c

decoder:
	mkdir -p $(BIN_FOLDER)
	gcc -std=c99 -o ./bin/decode.out ./src/serial/decode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c
	
build:
	mkdir -p $(BIN_FOLDER)
	mpicc -g -Wall -fopenmp -std=gnu99 -o ./bin/main.out ./src/parallel/main.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 

build_no_openmp:
	mkdir -p $(BIN_FOLDER)
	mpicc -g -Wall -std=gnu99 -o ./bin/main.out ./src/parallel/main.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 

data_generator:
	mkdir -p $(BIN_FOLDER)
	gcc -std=c99 -o ./bin/data_generator.out ./src/data_generator/data_generator.c