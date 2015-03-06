all: main.o
	gcc main.o -o quash -lreadline

main.o: main.c
	gcc -c -g main.c -lreadline

re: clean all
	

clean:
	rm -f *.o *.swp quash
