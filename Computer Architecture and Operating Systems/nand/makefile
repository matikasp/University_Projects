# compiler
CC = gcc

# options
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2

# linking options
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

# library name
LIBRARY = libnand.so

# source files
SOURCES = nand.c queue.c memory_tests.c

# header files
HEADERS = nand.h queue.h memory_tests.h

.PHONY: all clean

all: $(LIBRARY)

$(LIBRARY): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(LIBRARY) $(SOURCES)

clean:
	rm -f $(LIBRARY)
