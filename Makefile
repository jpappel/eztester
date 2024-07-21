CC := gcc
CFLAGS := -Wall
DEBUG_FLAGS := -ggdb -Og

SRCS := eztester.c
OBJS := eztester.o eztester_debug.o
STATIC_LIBS := libeztester.a libeztester_debug.a
DYNAMIC_LIBS := libeztester.a libeztester_debug.a

.PHONY: all static clean

all: $(STATIC_LIBS)

static: $(STATIC_LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -O3 -o $@ $<

%_debug.o: %.c
	$(CC) -c $(DEBUG_FLAGS) $(CFLAGS) -o $@ $<

lib%.a: %.o
	ar rcs $@ $<

clean:
	rm $(STATIC_LIBS) $(OBJS)
