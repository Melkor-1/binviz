CFLAGS += -O3
CFLAGS += -std=c11

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Warray-bounds
CFLAGS += -Wno-unused-value
CFLAGS += -Wno-type-limits
CFLAGS += -Werror
CFLAGS += -Wmissing-braces
CFLAGS += -Wno-parentheses
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wwrite-strings
CFLAGS += -Winline
CFLAGS += -Wno-unused
CFLAGS += -s

LDLIBS = -lm
TARGET = binviz

all: $(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: all clean
.DELETE_ON_ERROR:

