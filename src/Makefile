$BIN_FOLDER = ../bin

$(BIN_FOLDER)build counter: 
	@mpicc ./fileCounter/counterMain.c -o counter

$(BIN_FOLDER)build charCounter:
	@mpicc -o charCounterMPI ./fileCounter/charCounterMPI.c ./datastructures/dictionary.c ./datastructures/priorityQ.c 

$(BIN_FOLDER)build dictionary: 
	@gcc ./datastructures/dictionary.c -o dictionary

$(BIN_FOLDER)build priorityQ: 
	@gcc ./datastructures/priorityQ.c -o priority