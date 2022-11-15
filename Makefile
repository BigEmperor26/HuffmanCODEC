export LDFLAGS="-L/opt/homebrew/opt/libomp/lib"
export CPPFLAGS="-I/opt/homebrew/opt/libomp/include"

BIN_FOLDER=./bin/

encoder:
	gcc -std=c99 -o ./bin/encode.out ./src/serial/encode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c

decoder:
	gcc -std=c99 -o ./bin/decode.out ./src/serial/decode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c
	
build:
	mpicc -fopenmp -std=gnu99 -o ./bin/main.out ./src/parallel/main.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 

test:
	mpicc -fopenmp -std=gnu99 -o ./bin/test.out ./src/parallel/test.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 

folder:
	mpicc -fopenmp -std=gnu99 -o ./bin/folder.out ./src/parallel/filedistribution.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 
	
build no openmp:
	mpicc  -std=gnu99 -o ./bin/main.out ./src/parallel/main.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c ./src/parallel/processDistributer.c ./src/parallel/folder.c  ./src/parallel/encode.c ./src/parallel/decode.c 
