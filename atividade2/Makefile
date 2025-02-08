c:
	gcc -g -c core/modules/common/common.c
	gcc -g -c core/modules/commands/ls/ls.c
	gcc -g -c core/modules/commands/cd/cd.c
	gcc -g -c core/modules/commands/attr/attr.c
	gcc -g -c core/modules/commands/rename/rename.c
	gcc -g -c main.c
	gcc -o main *.o -lm
r:
	./main myimagefat32.img
cl:
	rm -rf *.o main
