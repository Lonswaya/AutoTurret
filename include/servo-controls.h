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

#define MAX_Y	180
#define MIN_Y	30
#define MAX_X	180
#define MIN_X	0

void go_to(int x, int y);
void go_to_smooth(int startX, int startY, int endX, int endY, int timems);
void *turn_smooth(void* arg/*int x, int y, int timems*/);
void *turn(void* arg/*int angle, int gpio*/);
int initialize();
