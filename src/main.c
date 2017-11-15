#include "Networking.h"
#include "MotionDetector.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum mode {
    MANUAL,
    AUTO
} Mode;

typedef struct user_configs {
    MotionConfig *motion_config;
    Mode mode;
    int sys;
    short move_x;
    short move_y;
} UserConfig;


int decode_packet(Packet *p, UserConfig *config) {
    switch(p->type) {
            
            case 0: //switching mode
                config->mode = (Mode) p->data;
                break;
            
            case 1: //manual control data: first 16 bits x, last 16 bits y
                config->move_y = (short) p->data;
                config->move_x = (short) (p->data >> 16);
                break;
            
            case 2: //control loop flag
                config->sys = 0;
                break;
            
            case 3:
                config->motion_config->motion_thresh = (short) p->data;
                config->motion_config->blur_size = (short) (p->data >> 16);
                break;

            default:
                return -1;                    
    }
}


int main(int argc, char **argv) {

    //---------- Detection thread ----------
    MotionConfig motion_config;
    motion_config.blur_size = 5;
    motion_config.motion_thresh = 10;

    MotionDetector md;
    md_init(&md, &motion_config);
    
    pthread_t detection_thread;

    if(pthread_create(&detection_thread, NULL, detection_loop, &md)) {
        printf("Err creating detection thread\n");
        exit(1);
    }
    

    //---------- network thread ----------
    Connection connection;
    memset(&connection, 0, sizeof(connection));
    net_init(&connection);
    pthread_t network_thread;
    
    if(pthread_create(&network_thread, NULL, net_loop, &connection)) {
        printf("Err creating network thread\n");
        exit(1);
    }
    printf("Network thread started.\n");

    //---------- control thread ----------
    //main becomes control thread

    UserConfig user_config;
    user_config.motion_config = &motion_config;
    user_config.mode = MANUAL;
    user_config.sys = 1;
    user_config.move_x = 0;
    user_config.move_y = 0;

    //packet buffer
    Packet packet;
    size_t size = 0;

    while(user_config.sys) {

        //attempt to decode a packet per iteration
        net_packetq_size(&connection, &size);
        if(size > 0) {
            //printf("Taking a packet...\n");
            if(net_get_packet(&connection, &packet) < 0) {
                printf("Err getting packet\n");
                //TODO: how to handle these errors
                continue;
            }           
            
            //printf("\tType: %d\n", packet.type);
            decode_packet(&packet, &user_config);
        }

        if(user_config.mode == MANUAL) {
            printf("MANUAL\n");
            printf("MOVE: (%hu, %hu)\n", user_config.move_x, user_config.move_y);
            
            //turn off detection
            user_config.motion_config->detect_flag = 0; 
            
            //TODO: 
            //code to move servo according to move_x and move_y (relative value to center of screen)
            //code to sleep appropreate amount of time until servo completed rotation

            //turn on detection
            user_config.motion_config->detect_flag = 1;

            continue;
        }

        if(user_config.mode == AUTO) {
            printf("AUTO\n");
            continue;
        }
    }

    //---------- control thread finished ----------
    //system shutdown kill all other threads

    connection.state = STOP;
    md.run_flag = 0;
    pthread_join(network_thread, NULL);  
    pthread_join(detection_thread, NULL);  
}
