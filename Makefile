BIN_FOLDER=./bin/

encoder:
	gcc -std=c99 -o ./bin/encode.out ./src/serial/encode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c

decoder:
	gcc -std=c99 -o ./bin/decode.out ./src/serial/decode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c
	
build:
	mpicc -g -Wall -fopenmp -std=gnu99 -o ./bin/main.out ./src/parallel/main.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 

test:
	mpicc -g -Wall -fopenmp -std=gnu99 -o ./test.out ./clusterscripts/test.c 

build_no_openmp:
	mpicc -g -Wall -std=gnu99 -o ./bin/main.out ./src/parallel/main.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 
