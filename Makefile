CC := gcc
CFLAGS := -Wall
DEBUG_FLAGS := -ggdb -Og

VERSION:= 0.1

BUILD_DIR := build
SRCS := eztester.c eztester_list.c
STATIC_OBJS := $(addprefix $(BUILD_DIR)/static/,$(SRCS:.c=.o))
DYNAMIC_OBJS := $(addprefix $(BUILD_DIR)/dynamic/,$(SRCS:.c=.o))
STATIC_LIBS := $(BUILD_DIR)/static/libeztester.a $(BUILD_DIR)/static/libeztester_debug.a
DYNAMIC_LIBS := $(BUILD_DIR)/dynamic/libeztester.so $(BUILD_DIR)/dynamic/libeztester_debug.so

all: $(STATIC_LIBS)

.PHONY: all static dynamic clean info

all: $(STATIC_LIBS) $(DYNAMIC_LIBS)

#static
static: $(STATIC_LIBS)

$(BUILD_DIR)/static/libeztester.a: $(STATIC_OBJS)
	ar rcs $@ $^

$(BUILD_DIR)/static/libeztester_debug.a: $(STATIC_OBJS:.c=_debug.o)
	ar rcs $@ $^

$(BUILD_DIR)/static/%.o: %.c $(BUILD_DIR)/static
	$(CC) -c $(CFLAGS) -O3 -o $@ $<

$(BUILD_DIR)/static/%_debug.o: %.c $(BUILD_DIR)/static
	$(CC) -c $(DEBUG_FLAGS) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/static:
	mkdir -p $@

# dynamic
dynamic: $(DYNAMIC_LIBS)

$(BUILD_DIR)/dynamic/libeztester.so: $(DYNAMIC_OBJS)
	$(CC) -shared -o $@ $^

$(BUILD_DIR)/dynamic/libeztester_debug.so: $(DYNAMIC_OBJS:.c=_debug.o)
	$(CC) -shared -o $@ $^

$(BUILD_DIR)/dynamic/%.o: %.c $(BUILD_DIR)/dynamic
	$(CC) -fpic -c $(CFLAGS) -O3 -o $@ $<

$(BUILD_DIR)/dynamic/%_debug.o: %.c $(BUILD_DIR)/dynamic
	$(CC) -fpic -c $(CFLAGS) $(DEBUG_FLAGS) -o $@ $<

$(BUILD_DIR)/dynamic:
	mkdir -p $@

info:
	@echo "SRCS: $(SRCS)"
	@echo "STATIC_OBJS: $(STATIC_OBJS)"
	@echo "STATIC_LIBS: $(STATIC_LIBS)"
	@echo "DYNAMIC_OBJS: $(DYNAMIC_OBJS)"
	@echo "DYNAMIC_LIBS: $(DYNAMIC_LIBS)"


clean:
	rm -rf $(BUILD_DIR)/static $(BUILD_DIR)/dynamic
