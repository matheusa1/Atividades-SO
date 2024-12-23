co:
	gcc -c modules/padawan/main.c -o modules/padawan/main.o
	gcc -c modules/publico/main.c -o modules/publico/main.o
	gcc -c modules/yoda/main.c -o modules/yoda/main.o
	gcc -c modules/common/main.c -o modules/common/main.o
	gcc main.c modules/padawan/main.o modules/publico/main.o modules/yoda/main.o modules/common/main.o -o main -lpthread
r:
	./main
cl:
	rm -rf modules/padawan/*.o
	rm -rf modules/publico/*.o
	rm -rf modules/yoda/*.o
	rm -rf modules/common/*.o
	rm -rf main
	rm -rf *.o