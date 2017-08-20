BIN = scheme.out

C_WARNINGS = -Wall -Wextra -Werror -Wno-unused-parameter
C_OPTIONS = -std=c99

C_LIBS = -lm

DEBUG_OPTIONS = -O0 -DDEBUG=1 -g -Wno-unused-function
RELEASE_OPTIONS = -O2

HEADERS := $(wildcard include/*.h src/*.h)
SOURCES := $(wildcard src/*.c)

.PHONY: test clean

release: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(RELEASE_OPTIONS) -Isrc/ -Iinclude/ $(SOURCES) -o $(BIN) $(C_LIBS)

debug: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(DEBUG_OPTIONS) -Isrc/ -Iinclude/ $(SOURCES) -o $(BIN) $(C_LIBS)

run: release
	./$(BIN)

valgrind: debug
	valgrind --leak-check=full ./$(BIN)

clean:
	rm -rf $(BIN)

format: 
	clang-format -i -style=file $(HEADERS) $(SOURCES)

test:
	test/test.py
