#include <stdio.h>
#include <stdlib.h>
#include "../include/servo-controller.h"
#include <pigpio.h>
#include <math.h>

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
	// These two gpio set modes are a workaround for updating position,
	// Something about the default timing is messed up if you don't gpioSetMode
	gpioSetMode(SERVO_X, PI_OUTPUT);
	gpioSetMode(SERVO_Y, PI_OUTPUT);	
	int result_x = gpioServo(SERVO_X, atous(data->xPos));
	int result_y = gpioServo(SERVO_Y, atous(data->yPos));
	if (result_x == PI_BAD_USER_GPIO) 
		printf("Bad user gpio: x (%d)\n", SERVO_X);
	if (result_y == PI_BAD_USER_GPIO) 
		printf("Bad user gpio: y (%d)\n", SERVO_Y);
	if (result_x == PI_BAD_PULSEWIDTH) 
		printf("Bad pulse width: x (%d)\n", atous(data->xPos));
	if (result_y == PI_BAD_PULSEWIDTH) 
		printf("Bad pulse width: y (%d)\n", atous(data->yPos));
	//printf("telling servos: %d, %d\n", atous(data->xPos), atous(data->yPos));
}

void servo_controller_init(struct servo_controller * sc) {
	sc->xPos = START_X;
	sc->yPos = START_Y;
	if (gpioInitialise() < 0) {
		printf("GPIO setup failed\n");
		exit(1);
	}

	update_servos(sc);
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
	//printf("controller update pos to %d, %d\n", sc->xPos, sc->yPos);
	update_servos(sc);
}
/**
 * Take in the input from the motion detector, and process it
 *
 */
void process_detected_input(struct servo_controller * sc, MotionDetector* md, UserConfig* user_config) {
	// Constants TODO use config values	
	
	double tracking_speed_x = -0.2;
	double tracking_speed_y = 0.6;

	double movement_threshold = 20; // How many degrees we can go before moving

	double recording_threshold = 40; // A threshold for recording the last positions
	
	
		
	double move_x = user_config->move_x * tracking_speed_x;
	double move_y = user_config->move_y * tracking_speed_y;
	
	double magnitude = sqrt(pow(move_x, 2) + pow(move_y, 2));
	if (md->strength > 1000) {
		printf("Strength: %d\n", md->strength);
	} else if (md->strength > 5000) {
		move_x *= log(md->strength) * 0.5;
		move_y *= log(md->strength) * 0.5;
	}
	if (magnitude > recording_threshold) {
		// ???
	}
	if (magnitude > movement_threshold) {
		servo_controller_turn(sc, move_x, move_y); 
	}
}
