#include <stdio.h>
#include <stdlib.h>
#include <curses.h>	// For getch
#include <signal.h>
#include "../include/servo-control.h"


/**
*
* This is the base class for human interaction (input, keystrokes) that apply to servos
*
*/
#define	SENSITIVITY	3
/* these are already defined in curses, but we redefine (cuz idk, workarounds?) */
#define MY_KEY_UP	65
#define MY_KEY_DOWN	66
#define MY_KEY_LEFT	68
#define MY_KEY_RIGHT	67


void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
	endwin();
	printf("SIGTERM received.\n");
}


void human_input_loop() {
	// SIGTERM handler
	static struct sigaction _sigact;

	memset(&_sigact, 0, sizeof(_sigact));
	_sigact.sa_sigaction = sig_term_handler;
	_sigact.sa_flags = SA_SIGINFO;

	sigaction(SIGTERM, &_sigact, NULL);

	int currentX = 90, currentY = 90; /* Keep track of where we are */
	// Start off at center, so we can track our input
	go_to(currentX, currentY);
	int c = 0;
	
	int diffX, diffY = 0;;
	initscr();
	timeout(-1);
	while (1) {
		// Take current human input, use it to turn in the desired direction
		c = 0;
		diffX = diffY = 0;
		// Break with a period
		switch((c=getch())) {
		case MY_KEY_UP:
			diffY = SENSITIVITY * 1;
			break;
		case MY_KEY_DOWN:
			diffY = SENSITIVITY * -1;
			break;
		case MY_KEY_RIGHT:
			diffX = SENSITIVITY * -1;
			break;
		case MY_KEY_LEFT:
			diffX = SENSITIVITY * 1;
			break;
		default:
			break;
		}
		//if (c != -1) printf("|%d|\r\n", c);	
		if (diffX != 0 || diffY != 0) {
			if (currentX + diffX >= MIN_X && currentX + diffX <= MAX_X) {
				currentX += diffX;
			}
			if (currentY + diffY >= MIN_Y && currentY + diffY <= MAX_Y) {
				currentY += diffY;
			}
			go_to(currentX, currentY);
		}
	}
	endwin();
}





