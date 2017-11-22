all: main

src/MotionDetector.o: src/MotionDetector.cpp
	g++ -c src/MotionDetector.cpp -lpthread `pkg-config --cflags --libs opencv`
src/servo-controls.o: src/servo-controls.c
	g++ -c src/servo-controls.c -Wall -pthread  -lwiringPi

src/servo-controller.o: src/servo-controller.c
	g++ -c src/servo-controller.c -Wall -pthread  -lwiringPi

main: src/Networking.c src/main.c src/MotionDetector.o src/servo-controller.o src/servo-controls.o
	g++ -o main src/main.c src/Networking.c MotionDetector.o servo-controller.o servo-controls.o -lwiringPi -Wall -lpthread -lpigpio `pkg-config --cflags --libs opencv` -g

clean:
	rm main servo-controls.o servo-controller.o MotionDetector.o
