#include <stdio.h>
#include <string.h>
#include "../include/servo-control.h"

void turn_hard_loop();
void turn_smooth_loop();

int main(int argc, char ** argv) {
	printf("Initiating auto tracker\n");
	
	// Initialization
	initialize();
	// Main loops
	// turn_hard_loop()
	if (argc > 1) {
		if (argv[1][0] == 'h') {
			human_input_loop();
		} 
	} else {
		turn_smooth_loop();
	}
}
void turn_hard_loop() {
	while (1) {
		// Swerve left to right
		go_to(90, 75);
		delay(2000);
		go_to(135, 90);
		delay(2000);
		go_to(180, 115);				
		delay(2000);
		go_to(135, 90);
		delay(2000);
	}
}
void turn_smooth_loop() {
	go_to(90, 90);
	while (1) {
		go_to_smooth(90, 90, 135, 135, 3000); 
		go_to_smooth(135, 135, 180, 90, 3000);
		go_to_smooth(180, 90, 135, 135, 3000);
		go_to_smooth(135, 135, 90, 90, 3000); 
	}
}
