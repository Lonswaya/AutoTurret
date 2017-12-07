#ifndef CONFIG_H_
#define CONFIG_H_

#include "../src/MotionDetector.h"

typedef enum mode {
    MANUAL,
    AUTO
} Mode;

typedef struct user_configs {
    MotionConfig *motion_config;
    Mode mode;
    short move_x;
    short move_y;
    unsigned short port;
    int start_x;
    int start_y;
    unsigned int timeout;
    unsigned int auto_freq;
    float x_sens;
    float y_sens;
    int s_min_x;
    int s_min_y;
    int s_max_x;
    int s_max_y; 
    char pass[16];
} UserConfig;

#endif
