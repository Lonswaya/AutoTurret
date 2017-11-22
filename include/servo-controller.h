
typedef struct servo_controller {
        int xPos;
	int yPos;
} servo_controller;

int atous(int angle);
void update_servos(void * userdata);
void servo_controller_init(struct servo_controller * sc);
void servo_controller_turn(struct servo_controller * sc, int diffX, int diffY);

