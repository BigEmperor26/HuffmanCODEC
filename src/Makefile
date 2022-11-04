$BIN_FOLDER = bin

$(BIN_FOLDER)build: counter.o
	@mpicc -o counter 

$(BIN_FOLDER)priorityQ : priorityQ.o
	@gcc -o priorityQ priorityQ.o

$(BIN_FOLDER)dictionary : dictionary.o
	@gcc -o dictionary dictionary.o

$(BIN_FOLDER)counter.o: ./fileCounter/counterMain.c
	@mpicc ./fileCounter/counterMain.c 