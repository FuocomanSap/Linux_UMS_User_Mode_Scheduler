#gcc test.c ./Headers/ums_user_library.c  -o test -lpthread 
CC=gcc
CFLAGS=-lpthread

all: test.c ./Headers/ums_user_library.c
	$(CC) test.c ./Headers/ums_user_library.c -o test $(CFLAGS)

clean:
	rm test