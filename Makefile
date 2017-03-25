BIN = scheme.out
C_WARNINGS = -Wall -Wextra -Werror
C_OPTIONS = -std=c99
C_LIBS = -lm

DEBUG_OPTIONS = -O0 -DDEBUG -g -Wno-unused-parameter -Wno-unused-function
RELEASE_OPTIONS = -O3 -Wno-unused-parameter

SOURCES := $(wildcard src/*.c)

debug: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(DEBUG_OPTIONS) $(C_LIBS) -Isrc/ $(SOURCES) -o $(BIN)

release: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(RELEASE_OPTIONS) $(C_LIBS) -Isrc/ $(SOURCES) -o $(BIN)

run: debug 
	./$(BIN)

valgrind: debug
	valgrind --leak-check=full ./$(BIN)

clean: 
	rm -rf $(BIN)
