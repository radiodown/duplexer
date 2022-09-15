CC = gcc
#CFLAGS = -DAPPLE
CFLAGS = -pthread
TARGET = duplexer
OBJECTS = duplexer.o init.o logger.o options.o master.o slave.o ping.o httpclient.o httpserver.o vip.o

all : $(TARGET)
$(TARGET) : $(OBJECTS)
		$(CC) $(CFLAGS) -o $@ $^

clean : 
	rm *.o duplexer
