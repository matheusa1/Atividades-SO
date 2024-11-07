array:
	gcc array.c -o array.exe -lpthread
matrix:
	gcc -c ./matriz/matriz.c -o matrix.o
	gcc createMatrix.c matrix.o -o createMatrix.exe
	gcc -c ./matriz/matriz.c -o matrix.o
	gcc matrix.c matrix.o -o matrix.exe -lpthread -lm
