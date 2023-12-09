# -*- MakeFile -*-


CC=gcc 
CFLAGS=-g -Wall 
TARGET=proiect
all:$(TARGET)
$(TARGET):
	$(CC) $(CFLAGS) -c processLib.c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c processLib.o
	rm *.o 

