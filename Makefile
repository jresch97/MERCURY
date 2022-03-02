CC = gcc
CFLAGS = -ansi -pedantic -Wall -O2
LDFLAGS = 

test: test.o parser.o
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm test test.o parser.o
