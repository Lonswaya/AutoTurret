NAME            = TrackerBot

CXX = g++ -g
RM=rm -f

# All source files we want to compile
SRCS=./src/tracker-loop.c ./src/servo-controls.c ./src/human-input.c ./src/servo-controller.c

# The object files from sources
OBJS=$(subst .c,.o,$(SRCS))

LDFLAGS=-Wall -pthread

LDLIBS= -lwiringPi -lncurses

all: tracker-loop

tracker-loop: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(NAME) $^ $(LDLIBS) 

tracker-loop.o : ./src/tracker-loop.c

servo-control.o : ./src/servo-controls.c ./include/servo-controls.h

servo-controller.o : ./src/servo-controller.c ./include/servo-controller.h

human-input.o : ./src/human-input.c ./include/human-input.h

clean:
	$(RM) $(OBJS)
distclean: clean
	$(RM) $(NAME)
