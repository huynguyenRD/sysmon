CC ?= $(TARGET_CC)
CFLAGS ?= -O2 -Wall
LDFLAGS ?= -lhal

SRCS = sysmon.c
OBJS = sysmon.o
TARGET = sysmon

all: ${TARGET}

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f sysmon $(OBJS)

.PHONY: all clean