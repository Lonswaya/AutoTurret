#include "../src/MotionDetector.h"

typedef enum mode {
    MANUAL,
    AUTO
} Mode;

typedef struct user_configs {
    MotionConfig * motion_config;
    Mode mode;
    int sys;
    short move_x;
    short move_y;
} UserConfig;
