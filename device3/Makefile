CC = cc
CFLAGS = -g
LIBS = -lpthread -lcjson
SRCS = main.c getLight.c led.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

depend: $(SRCS)
	makedepend $(SRCS)

-include $(SRCS:.c=.d)

.PHONY: all clean depend
