CC = gcc

CFLAGS = -g -Wall

TARGET = sfind

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lm

clean:
	$(RM) $(TARGET)
