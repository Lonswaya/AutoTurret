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
    MotionConfig motion_config;
    Mode mode;
    int sys;
} UserConfig;

/*
void *decod_loop(void *arg) {

}*/


int main(int argc, char **argv) {


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

    //---------- decoding thread ----------
    //main becomes decoding threa

    UserConfig user_config;
    user_config.mode = MANUAL;
    user_config.sys = 1;

    //packet buffer
    Packet packet;
    size_t size = 0;

    while(user_config.sys) {
        net_packetq_size(&connection, &size);
        if(size > 0) {
            printf("Taking a packet...\n");
            if(net_get_packet(&connection, &packet) < 0) {
                printf("Err getting packet\n");
                //TODO: how to handle these errors
                continue;
            }           
            
            printf("\tType: %d\n", packet.type);
            
        }
    }
}
