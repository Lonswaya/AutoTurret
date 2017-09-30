/* arguments to pass to a thread */
typedef struct turn_args {
	int pin;
	int deg;
} turn_args;

typedef struct turn_smooth_args {
	int pin;
	int start;
	int end;
	int timems;
} turn_smooth_args;



void go_to(int x, int y);
void go_to_smooth(int startX, int startY, int endX, int endY, int timems);
void turn_smooth(struct turn_smooth_args * args/*int x, int y, int timems*/);
void turn(struct turn_args * args/*int angle, int gpio*/);
int initialize();
