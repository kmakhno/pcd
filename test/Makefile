CC=gcc
CFLAGS=-Wall -g

llseek: llseek_test.o
	$(CC) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: llseek clean

clean:
	rm -f *.o llseek
