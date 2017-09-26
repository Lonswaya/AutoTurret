#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <unistd.h>

// Compile with  gcc -Wall -o blink blink.c -lwiringPi
/**
 * This is our basic servo interaction class with static interaction methods
*/

#define MAX_Y	180
#define MIN_Y	30
#define MAX_X	180
#define MIN_X	0
#define PIN_X	8
#define PIN_Y	7
#define SIGNAL_PULSES	50;

/* arguments to pass to a thread */
struct turn_args {
	int pin;
	int deg;
}

/**
 * Goes to the angles specified in a 2d 180 by 180 grid, if it is within bounds
 * Threaded GPIO usage to travel to coordinates
 * Assumes vertical gpio is #7, horizontal #8
*/
// pthread_mutex_t mutex
void go_to(int x, int y) {
	if (x < MIN_X || x > MAX_X) {
		printf("Unable to travel to coordinates, X must be in range [%d, %d], but was %d\n",MIN_X, MAX_X, x);
		return;
	}
	if (y < MIN_Y || y > MAX_Y) {
		printf("Unable to travel to coordinates, Y must be in range [%d, %d], but was %d\n",MIN_Y, MAX_Y), y;
		return;
	}
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
	pthread_create( &s_x, &attr, (void * (*)(void *)) turn, (void *) x_args);
	pthread_create( &s_y, &attr, (void * (*)(void *)), turn, (void *) y_args);

	// Wait for both threads to finish
	pthread_join(s_x, NULL);
	pthread_join(s_y, NULL);

	printf("Traveled to (%d, %d)\n", x, y);
}

void turn(turn_args args) {
	if(args.deg < 30 || args.deg >= 180) {
		print("wrong angle\n");
		return;
	}

	float tmp = ( (float) 2 / 180) * args.deg;
	tmp *= 1000;
	int tmp2 = (int) tmp;

	printf("%d\n",tmp2);
	int i;
	for(i = 0; i < SIGNAL_PULSES; i++) {
		digitalWrite(args.pin, 1);
		delayMicroseconds(tmp2);
		digitalWrite(arsg.pin, 0);
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
