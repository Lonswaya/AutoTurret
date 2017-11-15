all: main

MotionDetector.o: MotionDetector.cpp
	g++ -c MotionDetector.cpp `pkg-config --cflags --libs opencv`


main: Networking.c main.c MotionDetector.o
	g++ -o main main.c Networking.c MotionDetector.o -Wall -lpthread `pkg-config --cflags --libs opencv` -g

clean:
	rm main MotionDetector.o
