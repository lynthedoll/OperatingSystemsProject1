shell: shell.c
	gcc -Wall -g -std=c99 -o shell shell.c

clean:
	rm -f shell