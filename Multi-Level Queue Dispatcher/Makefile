
SRC = multi_level.c pcb.c
HEADERS = multi_level.h pcb.h

all: multi_level process

multi_level: $(SRC) $(HEADERS)
	gcc $(SRC) -o $@

process: sigtrap.c
	gcc $< -o $@

clean:
	rm -f multi_level process