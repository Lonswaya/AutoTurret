#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <math.h>

// Compile with  gcc -Wall -o blink blink.c -lwiringPi
/**
 * This is our basic servo interaction class with static interaction methods
*/

#define MAX_Y	180
#define MIN_Y	30
#define MAX_X	180
#define MIN_X	0
#define PIN_X	7
#define PIN_Y	11
#define SIGNAL_PULSES	2


/* arguments to pass to a thread */
typedef struct turn_args {
	int pin;
	int deg;
} turn_args;

void turn(int pin, int deg);
void turn_smooth(int pin, int start, int end, int timems);
/**
 * Goes to the angles specified in a 2d 180 by 180 grid, if it is within bounds
 * Threaded GPIO usage to travel to coordinates
 * Assumes vertical gpio is #7, horizontal #8
*/
void go_to(int x, int y) {
	if (x < MIN_X || x > MAX_X) {
		printf("Unable to travel to coordinates, X must be in range [%d, %d], but was %d\n",MIN_X, MAX_X, x);
		return;
	}
	if (y < MIN_Y || y > MAX_Y) {
		printf("Unable to travel to coordinates, Y must be in range [%d, %d], but was %d\n",MIN_Y, MAX_Y), y;
		return;
	}
	int pid2 = fork();
	if (pid2 == 0) {
		// Child thread
		// Handle writing to X servo
		turn(PIN_X, x);
		exit(0);
	} else {
		// Handle writing to Y servo
		turn(PIN_Y, y);
		// Wait for the other process to finish
		wait();
	}
	printf("Traveled to (%d, %d)\n", x, y);
}
/**
 * Will turn as smooth as possible from the start point to the end points given
*/
void go_to_smooth(int startX, int startY, int endX, int endY, int timems) {
	if (endX < MIN_X || endX > MAX_X) {
		printf("Unable to travel to coordinates, X must be in range [%d, %d], but was %d\n",MIN_X, MAX_X, endX);
		return;
	}
	if (endY < MIN_Y || endY > MAX_Y) {
		printf("Unable to travel to coordinates, Y must be in range [%d, %d], but was %d\n",MIN_Y, MAX_Y), endY;
		return;
	}
	int pid2 = fork();
	if (pid2 == 0) {
		// Child thread
		// Handle writing to X servo
		turn_smooth(PIN_X, startX, endX, timems);
		exit(0);
	} else {
		// Handle writing to Y servo
		turn_smooth(PIN_Y, startY, endY, timems);
		// Wait for the other process to finish
		wait();
	}
	printf("Turned smoothly to (%d, %d)\n", endX, endY);
}

void turn_smooth(int pin, int start, int end, int timems) {
	if (timems == 0) {
		printf("We're not that fast, sucker. Use turn(int pin, int deg) instead\n");
		return;
	}
	turn(pin, start);
	int steps = timems/(SIGNAL_PULSES * 20); // The amount of steps that we need to take from here to there
	float step_deg = ((float)(end-start))/(steps);
	//printf("Taking %d steps, going %f each step to reach %d degrees\n", steps, step_deg, end);
	int currentAngle = start;
	//printf("abs: %d\n", abs(currentAngle - end));
	while (abs(currentAngle - end) > abs(ceil(step_deg))) { 
		if (pin == 7) {
			//printf("abs: %d, step: %f, pos: %d\n", abs(currentAngle-end), step_deg, currentAngle);
		}
		currentAngle += (int)roundf(step_deg);
		turn(pin, currentAngle);
	}
	// Close enough, close the gap
	turn(pin, end);
	
}
void turn(int pin, int deg) {
	if(deg < 30 || deg > 180) {
		//printf("wrong angle, value is %d when we expect to be 30 < deg < 180\n", deg);
		return;
	}

	// Magic bullshit to find our turn angle
	float tmp = ( (float) 2 / 180) * deg;
	tmp *= 1000;
	int tmp2 = (int) tmp;

	//printf("%d\n",tmp2);
	int i;
	const unsigned int PULSE = SIGNAL_PULSES;
	for(i = 0; i < PULSE; i++) {
		digitalWrite(pin, 1);
		delayMicroseconds(tmp2);
		digitalWrite(pin, 0);
		delayMicroseconds(20000 - tmp2);
	}		
}

int initialize() {
	return wiringPiSetup();
	pinMode(PIN_X, OUTPUT);
	pinMode(PIN_Y, OUTPUT);
}

/*
int main(void) {
	printf("Raspberry Pi blink\n");
	if (wiringPiSetup() == -1) return 1;
	
	turn(30, 7);
	delay(1000);
	turn(29, 7);
	delay(1000);
	turn(28, 7);
	delay(1000);
	turn(27, 7);
	delay(1000);
	turn(26, 7);
	delay(1000);
	turn(25, 7);
}*/
