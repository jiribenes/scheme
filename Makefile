BIN = scheme
C_WARNINGS = -Wall -Wextra -Werror
C_OPTIONS = -std=c99
DEBUG_OPTIONS = -O0 -DDEBUG -g -std=c99 -Wno-unused-parameter -Wno-unused-function

HEADERS := $(wildcard src/*.h)
SOURCES := $(wildcard src/*.c)

debug: $(SOURCES)
	$(CC) -Isrc/ $(DEBUG_OPTIONS) $(C_WARNINGS) $(SOURCES) -o $(BIN) -lm

clean: 
	rm -rf $(BIN)
