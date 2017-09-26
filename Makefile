# -lpthread
# gcc -Wall -o blink blink.c -lwiringPi
NAME            = TrackerBot

CXX = g++ -g
RM=rm -f

# All source files we want to compile
SRCS=src/tracker-loop.c src/servo-control.c

# The object files from sources
OBJS=$(subst .c,.o,$(SRCS))

# TODO add flags
LDFLAGS=-Wall

# TODO add libraries
LDLIBS= -lpthread, -lwiringPi

all: tracker-loop

tracker-loop:$(OBJS)
	$(CXX) $(LDFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)

tracker-loop.o : include/servo-control.h src/servo-control.c

clean:
	$(RM) $(OBJS)
distclean: clean
	$(RM) $(NAME)
