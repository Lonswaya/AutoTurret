#include <stdio.h>
#include <stdlib.h>
#include "../include/servo-controller.h"
#include "../include/servo-controls.h"

/**
 * This is our high-level servo abstraction
 * That maintains servo position and uses the difference to move the components
 */

/**
 * Update servo positions using the passed servo controller
 */
void update_servos(struct servo_controller * sc) {
	go_to(sc->xPos, sc->yPos);
}

void servo_controller_init(struct servo_controller * sc) {
	initialize();
	sc->xPos = 90;
	sc->yPos = 90;
	update_servos(sc);
}

/**
 * Turns and updates the given SC to have the new values passed
 */
void servo_controller_turn(struct servo_controller * sc, int diffX, int diffY) {
	if (sc->xPos + diffX >= MAX_X) {
		sc->xPos = MAX_X;
	} else if (sc->xPos + diffX <= MIN_X) {
		sc->yPos = MIN_X;
	} else {
		sc->xPos += diffX;
	}

	if (sc->yPos + diffY >= MAX_Y) {
		sc->yPos = MAX_Y;
	} else if (sc->yPos + diffY <= MIN_Y) {
		sc->yPos = MIN_Y;
	} else {
		sc->yPos += diffY;
	}

	update_servos(sc);
}
