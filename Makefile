all:
	gcc main.c -o CosSim -ansi -lm
clean:
	rm -rf main.c CosSim Makefile
