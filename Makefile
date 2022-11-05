export LDFLAGS="-L/opt/homebrew/opt/libomp/lib"
export CPPFLAGS="-I/opt/homebrew/opt/libomp/include"

BIN_FOLDER=./bin/

test:
	echo LDFLAGS
counter: 
	@mpicc -o $(BIN_FOLDER)/$ counter ./src/fileCounter/counterMain.c  ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c

threadCounter:
	@mpicc -o $(BIN_FOLDER)/$ threadCounter ./src/parallel/charCounterThread.c  ./src/datastructures/priorityQ.c ./src/datastructures/dictionary.c -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp

fileProcesser:
	@mpicc -fopenmp -std=c99 -o ./bin/fileProcesser ./src/parallel/fileProcesser.c  ./src/datastructures/dictionary.c 