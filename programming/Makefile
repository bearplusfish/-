# c ompileer
CC=gcc
# report all warnings, link with lib pthread
CFLAGS=-Wall -lpthread

baber_shop: baber_shop.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f ./baber_shop
