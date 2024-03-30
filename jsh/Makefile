CC = clang-16
CFLAGS = -std=gnu17 -Wvla -Wall -Wextra -Werror -Wpedantic -Wno-unused-result -Wconversion
JSH_SRC = jsh.c
JSH = jsh 
JSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
JSHMC = jsh_memory_check
.PHONY: clean

all: $(JSH) $(JSHHMC)
	@echo jsh successfully constructed

$(JSH): $(JSH_SRC) 
	$(CC) $(CFLAGS) -o $(JSH) $(JSH_SRC)

$(JSHMC) : $(JSH_SRC) 
	$(CC) $(CFLAGS) $(JSHMC_FLAGS) -o $(JSHMC) $(JSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@ 

clean: 
	$(RM) *.o *.a *~ $(JSH) $(JSHMC)