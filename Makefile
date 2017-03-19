BIN = scheme.out
C_WARNINGS = -Wall -Wextra -Werror
C_OPTIONS = -std=c99
C_LIBS = -lm

DEBUG_OPTIONS = -O0 -DDEBUG -g -Wno-unused-parameter -Wno-unused-function
RELEASE_OPTIONS = -O3

HEADERS := $(wildcard src/*.h)
SOURCES := $(wildcard src/*.c)

debug: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(DEBUG_OPTIONS) $(HEADERS) $(SOURCES) -o $(BIN) $(C_LIBS)

release: $(SOURCES)
	$(CC) $(C_OPTIONS) $(C_WARNINGS) $(RELEASE_OPTIONS) $(HEADERS) $(SOURCES) -o $(BIN) $(C_LIBS)

run: 
	./$(BIN)

clean: 
	rm -rf $(BIN)
