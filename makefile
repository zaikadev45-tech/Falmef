C = gcc
CFLAGS = -Wall -Wextra -Wpedantic -I./include -O3 -DNDEBUG -fdata-sections -ffunction-sections
LDFLAGS = -flto -Wl,--gc-sections

SRCS = main.c src/input.c src/modulos/TCP_connect.c src/modulos/SYN_stealth.c
OBJS = $(SRCS:.c=.o)
	TARGET = falmef

all: $(TARGET)

$(TARGET): $(OBJS)
		$(CC) $(OBJS) -o $@ $(LDFLAGS)
		@echo "\n[+] Falmef foi contruido com sucesso!"

%.o: %.c
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -f $(OBJS) $(TARGET)

.PHONY: all clean
