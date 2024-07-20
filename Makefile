CC := gcc
CFLAGS := -Wall

SRCS := eztester.c
OBJS := eztester.o eztester_debug.o
STATIC_LIBS := libeztester.a libeztester_debug.a

.PHONY: all clean

all: $(STATIC_LIBS)

eztester.o: eztester.c
	$(CC) -c $(CFLAGS) -O3 -o $@ $<

eztester_debug.o: eztester.c
	$(CC) -c -ggdb -Og $(CFLAGS) -o $@ $<

lib%.a: %.o
	ar rcs $@ $<

clean:
	rm $(STATIC_LIBS) $(OBJS)
