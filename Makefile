CC := gcc
CFLAGS := -Wall

SRCS := eztester.c
OBJS := eztester.o eztester_debug.o
STATIC_LIBS := eztester.a eztester_debug.a

all: $(STATIC_LIBS)

eztester.o: eztester.c
	$(CC) -c $(CFLAGS) -o $@ $<

eztester_debug.o: eztester.c
	$(CC) -c -ggdb $(CFLAGS) -o $@ $<

%.a: %.o
	ar rcs $@ $<
