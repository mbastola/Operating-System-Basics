
CC = gcc
CFLAGS = -Wall -g	
DEPS = xssh.h uthash.h
OBJ = xssh.o

xssh.o: xssh.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

xssh: $(OBJ)
	gcc $(CFLAGS) -o $@ $^
 
clean:
	rm -f xssh *.o *~
