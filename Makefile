all: shell

shell: shell.c
	gcc -o shell shell.c -lpthread

clean:
	rm -f shell
