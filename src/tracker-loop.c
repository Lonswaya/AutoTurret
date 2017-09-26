#include <stdio.h>
#include <string.h>
#include "servo-control.h"

int main(void) {
	printf("Initiating auto tracker");
	
	// Initialization
	initialize();
	// Main loops
	while(1) {
		// Swerve left to right
		go_to(30, 90);
		go_to(180, 90);				
	}
}
