CC = gcc
#CFLAGS = -DAPPLE
TARGET = duplexer
OBJECTS = duplexer.o init.o logger.o options.o master.o ping.o httpclient.o httpserver.o vip.o

all : $(TARGET)
$(TARGET) : $(OBJECTS)
		$(CC) $(CFLAGS) -o $@ $^

clean : 
	rm *.o duplexer
