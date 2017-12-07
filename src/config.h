#ifndef CONFIG_H_
#define CONFIG_H_

typedef enum mode {
    MANUAL,
    AUTO
} Mode;

typedef struct user_configs {
    MotionConfig *motion_config;
    Mode mode;
    short move_x;
    short move_y;
    unsigned int port;
    unsigned int start_x;
    unsigned int start_y;
    unsigned int timeout;
    unsigned int auto_freq;
    unsigned int x_sens;
    unsigned int y_sens;
    unsigned int s_min_x;
    unsigned int s_min_y;
    unsigned int s_max_x;
    unsigned int s_max_y; 
    char pass[16];
} UserConfig;

#endif
