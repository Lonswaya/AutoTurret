
#define	SERVO_X	11
#define	SERVO_Y	9

#define	START_X	75
#define	START_Y	-15

#define	MIN_X	-40
#define	MAX_X	250

#define	MIN_Y	-90
#define	MAX_Y	180

typedef struct servo_controller {
        int xPos;
	int yPos;
} servo_controller;

int atous(int angle);
void update_servos(void * userdata);
void servo_controller_init(struct servo_controller * sc);
void servo_controller_turn(struct servo_controller * sc, int diffX, int diffY);

