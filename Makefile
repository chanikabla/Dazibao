-CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lm
OFILES = pair.o main.o util.o sha256.o node.o neighbour.o

DEPS = pair.h util.h sha256.h node.h neighbour.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OFILES)
	$(CC) -o dazibao $(OFILES)

clean:
	rm *.o *_trace.txt *~

clearall: 
	rm -rf *~ $(ALL)
