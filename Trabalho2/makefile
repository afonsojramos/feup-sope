CC = gcc

CFLAGS = -g -Wall

TARGET = gerador
SECOND = sauna

all: $(TARGET) $(SECOND)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lpthread
	
$(SECOND): $(SECOND).c
	$(CC) $(CFLAGS) -o $(SECOND) $(SECOND).c -lpthread
 
	
clean:
	$(RM) $(TARGET) 
	$(RM) $(SECOND)