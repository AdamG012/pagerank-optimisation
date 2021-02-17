CC=gcc
CFLAGS=-g -std=c11 -Wall -Werror -O0 -fopenmp
TARGET=program
.PHONY: clean
all: $(TARGET)

pagerank: src/pagerank.c src/pagerank.h
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm

test_pagerank: test/test_pagerank.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcmocka

clean:
	rm -f *.o
	rm -f pagerank
	rm -f test_pagerank
