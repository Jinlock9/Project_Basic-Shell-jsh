CC = clang
CFLAGS = -std=gnu17 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result -Wconversion -Wvla
JSH_SRC = *.c
JSH = jsh
JSHMC = jsh_memory_check
JSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
.PHONY: clean

all: $(JSH) $(JSHMC)
	@echo JSH successfully constructed

$(JSH): $(JSH_SRC)
	$(CC) $(CFLAGS) -o $(JSH) $(JSH_SRC)

$(JSHMC) : $(JSH_SRC)
	$(CC) $(CFLAGS) $(JSHMC_FLAGS) -o $(JSHMC) $(JSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(JSH) $(JSHMC)