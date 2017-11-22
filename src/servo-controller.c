#include <stdio.h>
#include <stdlib.h>
#include "../include/servo-controller.h"
#include "../include/servo-controls.h"
#include <pigpio.h>

/**
 * This is our high-level servo abstraction
 * That maintains servo position and uses the difference to move the components
 */

/**
 * Angle to microseconds
 */
int atous(int angle) {
	float period_ms = ( (float) 1/ 180) * angle;
	period_ms *= 1000; // Convert to microseconds
	period_ms += 1000; // Add initial 1000 us period
	return period_ms;
}

/**
 * Update servo positions using the passed servo controller
 * This can be called by gpioSetTimerFuncEx
 * We take a void pointer for the timer function, but expect servo controller data 
 */
void update_servos(void * userdata) {
	struct servo_controller * data  = (struct servo_controller *) userdata;
	gpioServo(7, atous(data->xPos));
	gpioServo(0, atous(data->yPos));
	//go_to(sc->xPos, sc->yPos);
}

void servo_controller_init(struct servo_controller * sc) {
	//initialize();
	sc->xPos = 90;
	sc->yPos = 90;
	gpioInitialise();
	gpioSetMode(7, PI_OUTPUT);
	gpioSetMode(0, PI_OUTPUT);
	printf("servo controller set up\n");
}

/**
 * Turns and updates the given SC to have the new values passed
 */
void servo_controller_turn(struct servo_controller * sc, int diffX, int diffY) {
	int xMaxHit, yMaxHit;

	if (sc->xPos + diffX >= MAX_X) {
		sc->xPos = MAX_X;
		xMaxHit = 1;
	} else if (sc->xPos + diffX <= MIN_X) {
		sc->xPos = MIN_X;
		xMaxHit = 1;
	} else {
		sc->xPos += diffX;
	}

	if (sc->yPos + diffY >= MAX_Y) {
		sc->yPos = MAX_Y;
		yMaxHit = 1;
	} else if (sc->yPos + diffY <= MIN_Y) {
		sc->yPos = MIN_Y;
		yMaxHit = 1;
	} else {
		sc->yPos += diffY;
	}
	if (xMaxHit && yMaxHit) return;
	printf("controller update pos to %d, %d\n", sc->xPos, sc->yPos);
}
