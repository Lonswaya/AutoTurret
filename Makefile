# -lpthread
# gcc -Wall -o blink blink.c -lwiringPi


NAME	= TrackerBot

CC 	= 	gcc

CFLAGS 	=	-Wall -lpthread -lwiringPi

TARGET	=	src/tracker-loop.c

all:	$(TARGET)

$(TARGET):	$(TARGET).c
	$(CC) $(CFLAGS) -o $(NAME) $(TARGET)

clean:
	$(RM) $(NAME)
