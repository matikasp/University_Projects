# kompilator
CC = gcc

# opcje
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2

# opcje linkowania
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

# nazwa biblioteki
LIBRARY = libnand.so

# pliki zrodlowe
SOURCES = nand.c queue.c memory_tests.c

# pliki naglowkowe
HEADERS = nand.h queue.h memory_tests.h

.PHONY: all clean

all: $(LIBRARY)

$(LIBRARY): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(LIBRARY) $(SOURCES)

clean:
	rm -f $(LIBRARY)
