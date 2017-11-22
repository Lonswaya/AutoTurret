all: main

MotionDetector.o: MotionDetector.cpp
	g++ -c MotionDetector.cpp -lpthread `pkg-config --cflags --libs opencv`
servo-controls.o: servo-controls.c
	g++ -c servo-controls.c -Wall -pthread  -lwiringPi

servo-controller.o: servo-controller.c
	g++ -c servo-controller.c -Wall -pthread  -lwiringPi

main: Networking.c main.c MotionDetector.o servo-controller.o servo-controls.o
	g++ -o main main.c Networking.c MotionDetector.o servo-controller.o servo-controls.o -lwiringPi -Wall -lpthread -lpigpio `pkg-config --cflags --libs opencv` -g

clean:
	rm main MotionDetector.o
