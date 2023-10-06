CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lsqlite3

SRCS = server.c file.c utils.c sql.c usermenu.c
OBJS = $(SRCS:.c=.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
