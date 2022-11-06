export LDFLAGS="-L/opt/homebrew/opt/libomp/lib"
export CPPFLAGS="-I/opt/homebrew/opt/libomp/include"

BIN_FOLDER=./bin/

test:
	echo LDFLAGS

fileProcesser:
	@mpicc -fopenmp -std=c99 -o ./bin/fileProcesser.out ./src/parallel/fileProcesser.c  ./src/datastructures/dictionary.c 

encoder:
	gcc -std=c99 -o ./bin/encode.out ./src/serial/encode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/serial/huffman.c
	
parallelEncoder:
	@mpicc -fopenmp -std=c99 -o ./bin/parallelEncode.out ./src/parallel/encode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c

parallelDecoder:
	@mpicc -fopenmp -std=c99 -o ./bin/parallelDecode.out ./src/parallel/decode.c ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c ./src/parallel/huffman.c  ./src/parallel/frequency.c

writer:
	@gcc -std=c99 -o ./bin/writer.out ./src/writer.c