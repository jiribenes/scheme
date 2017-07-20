BIN = scheme.out
C_WARNINGS = -Wall -Wextra -Werror
C_OPTIONS = -std=c99
C_LIBS = -lm

DEBUG_OPTIONS = -O0 -DDEBUG -g -Wno-unused-parameter -Wno-unused-function
RELEASE_OPTIONS = -O3 -Wno-unused-parameter

HEADERS := $(wildcard include/*.h src/*.h)
SOURCES := $(wildcard src/*.c)

.PHONY: test clean

debug: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(DEBUG_OPTIONS) $(C_LIBS) -Isrc/ -Iinclude/ $(SOURCES) -o $(BIN)

release: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(RELEASE_OPTIONS) $(C_LIBS) -Isrc/ -Iinclude/ $(SOURCES) -o $(BIN)

run: debug 
	./$(BIN)

valgrind: debug
	valgrind --leak-check=full ./$(BIN)

clean:
	rm -rf $(BIN)

# TODO: modify this to use SOURCES :)
format: 
	clang-format -i -style=file $(HEADERS) $(SOURCES)

test:
	test/test.py
