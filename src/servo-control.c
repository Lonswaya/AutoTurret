#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include "../include/servo-control.h"

// Compile with  gcc -Wall -o blink blink.c -lwiringPi
/**
 * This is our basic servo interaction class with static interaction methods
*/

#define MAX_Y	180
#define MIN_Y	30
#define MAX_X	180
#define MIN_X	0
#define PIN_X	7
#define PIN_Y	0
#define SIGNAL_PULSES	2


//void turn(struct * turn_args args/*int pin, int deg*/);
//void turn_smooth(struct * turn_smooth_args args/*int pin, int start, int end, int timems*/);

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
	/*int pid2 = fork();
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
	}*/
	pthread_t s_x, s_y;
	pthread_attr_t attr;
	// We may depend on a mutex lock, depending if power issues are a concern
	//pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	struct turn_args x_args, y_args;
	x_args.pin = PIN_X;
	y_args.pin = PIN_Y;
	x_args.deg = x;
	y_args.deg = y; 
	pthread_create( &s_x, &attr, (void *) &turn, (void *) &x_args);
	pthread_create( &s_y, &attr, (void *) &turn, (void *) &y_args);

	// Wait for both threads to finish
	pthread_join(s_x, NULL);
	pthread_join(s_y, NULL);
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
	/*int pid2 = fork();
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
	}*/
	pthread_t s_x, s_y;
	pthread_attr_t attr;
	// We may depend on a mutex lock, depending if power issues are a concern
	//pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	//pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	struct turn_smooth_args x_args, y_args;
	x_args.pin = PIN_X;
	y_args.pin = PIN_Y;
	x_args.start = startX;
	y_args.start = startY;
	x_args.end = endX;
	y_args.end = endY;
	x_args.timems = timems;
	y_args.timems = timems;
	printf("pin: %d, start: %d, end: %d, timems: %d\n",x_args.pin, x_args.start, x_args.end, x_args.timems);
	printf("pin: %d, start: %d, end: %d, timems: %d\n",y_args.pin, y_args.start, y_args.end, y_args.timems);
	pthread_create( &s_x, /*&attr*/ NULL, (void *) &turn_smooth, (void *) &x_args);
	pthread_create( &s_y, /*&attr*/ NULL, (void *) &turn_smooth, (void *) &y_args);

	// Wait for both threads to finish
	pthread_join(s_x, NULL);
	pthread_join(s_y, NULL);
	printf("Turned smoothly to (%d, %d)\n", endX, endY);
}

void turn_smooth(struct turn_smooth_args * args/*int pin, int start, int end, int timems*/) {
	//printf("pin: %d, start: %d, end: %d, timems: %d\n",args->pin, args->start, args->end, args->timems);
	if (args->timems == 0) {
		printf("We're not that fast, sucker. Use turn(int pin, int deg) instead\n");
		return;
	}
	struct turn_args args2;
	args2.pin = args->pin;
	args2.deg = args->start;
	turn(&args2);
	int steps = args->timems/(SIGNAL_PULSES * 20); // The amount of steps that we need to take from here to there
	float step_deg = ((float)(args->end-args->start))/(steps);
	//printf("Taking %d steps, going %f each step to reach %d degrees\n", steps, step_deg, args->end);
	int currentAngle = args->start;
	//printf("abs: %d\n", abs(currentAngle - args->end));
	while (abs(currentAngle - args->end) > abs(ceil(step_deg))) { 
		if (args->pin == 7) {
			printf("abs: %d, step: %f, pos: %d\n", abs(currentAngle-args->end), step_deg, currentAngle);
		}
		currentAngle += (int)roundf(step_deg);
		args2.pin = args->pin;
		args2.deg = currentAngle;
		turn(&args2);
	}
	// Close enough, close the gap
	args2.pin = args->pin;
	args2.deg = args->end;
	turn(&args2);
	
}
void turn(struct turn_args * args/*int pin, int deg*/) {
	if(args->deg < 30 || args->deg > 180) {
		//printf("wrong angle, value is %d when we expect to be 30 < deg < 180\n", args->deg);
		return;
	}

	// Magic bullshit to find our turn angle
	float tmp = ( (float) 2 / 180) * args->deg;
	tmp *= 1000;
	int tmp2 = (int) tmp;

	//printf("%d\n",tmp2);
	int i;
	const unsigned int PULSE = SIGNAL_PULSES;
	for(i = 0; i < PULSE; i++) {
		digitalWrite(args->pin, 1);
		//usleep(tmp2 * 1000);
		delayMicroseconds(tmp2);
		digitalWrite(args->pin, 0);
		//usleep((20000 - tmp2) * 1000);
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
