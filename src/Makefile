all: main

MotionDetector.o: MotionDetector.cpp
	g++ -c MotionDetector.cpp `pkg-config --cflags --libs opencv`

main.o: main.c
	g++ -c main.c

main: MotionDetector.o main.o
	g++ -o main main.o MotionDetector.o

clean:
	rm main main.o MotionDetector.o
