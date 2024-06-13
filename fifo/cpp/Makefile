CC=g++
OUT=fifo

CFLAGS=-Wall\
	   -Wextra\
	   -Wconversion\
	   -Wpedantic\
	   -Werror\
	   -std=c++23\
	   -ggdb3\
	   -O0\
	   -fsanitize=undefined,address
LDFLAGS=-fsanitize=undefined,address

INCLUDE_DIR=include

SRC=main.cpp
OBJ=$(SRC:.cpp=.o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $(OUT) $(LDFLAGS) $(OBJ)

$(OBJ): $(SRC)
	$(CC) -c $(CFLAGS) -I$(INCLUDE_DIR) $(SRC)

clean:
	rm -f $(OBJ) $(OUT)

.PHONY: all clean
