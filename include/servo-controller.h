
typedef struct servo_controller {
        int xPos;
	int yPos;
} servo_controller;

int xPos, yPos;
void servo_controller_init(struct servo_controller * sc);
void servo_controller_turn(struct servo_controller * sc, int diffX, int diffY);

