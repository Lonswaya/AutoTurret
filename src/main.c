#include "Networking.h"
#include "MotionDetector.h"
#include "../include/servo-controller.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pigpio.h>
#include <unistd.h>
#include <math.h>

/*
int decode_packet(Packet *p, UserConfig *config) {
    switch(p->type) {
            
            case 0: //switching mode
                config->mode = (Mode) p->data;
                config->move_x = 0;
                config->move_y = 0;

                //TODO:switching to auto mode should clock the time and clear motion detector's center_of_motion buffer
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
}*/

/**
 * Moves the servos based off of the user config
 */
void MoveServos(UserConfig user_config, servo_controller sc, int sensitivity) {
	int m_x, m_y;
	if(user_config.move_y == 2) {
		m_y = sensitivity;
	}else if(user_config.move_y == 1) { 
		m_y = -1 * sensitivity;
	}
	if (user_config.move_x == 2) {
		m_x = sensitivity;
	} else if (user_config.move_x == 1) {
		m_x = -1 * sensitivity;
	}
	servo_controller_turn(&sc, m_x, m_y);
}



/* 
 * Populate UserConfig from a file
 */
int load_config(UserConfig *c) {
    FILE *file = fopen("config.cfg", "r");
    if(file == NULL) {
        return -1;
    }
    
    char buf[64];

    int ch;
    int buf_idx = 0;
    int g_idx = 0;

    while((ch = fgetc(file)) != EOF) {
        if(ch == '\n') {
            buf[buf_idx] = '\0';
            
            if(g_idx == 0) {
                c->mode = (Mode) strtol(buf);
            } else if(g_idx == 1) {
                c->move_x = (short) strtol(buf);
            } else if(g_idx == 2) {
                c->move_y = (short) strtol(buf);
            } else if(g_idx == 3) {
                c->port = strtoul(buf);
            } else if(g_idx == 4) {
                c->start_x = strtoul(buf);
            } else if(g_idx == 5) {
                c->start_y = strtoul(buf);
            } else if(g_idx == 6) {
                c->timeout = strtoul(buf);
            } else if(g_idx == 7) {
                c->auto_freq = strtoul(buf);
            } else if(g_idx == 8) {
                c->x_sens = strtoul(buf);
            } else if(g_idx == 9) {
                c->y_sens = strtoul(buf);
            } else if(g_idx == 10) {
                c->s_min_x = strtoul(buf);
            } else if(g_idx == 11) {
                c->s_min_y = strtoul(buf);
            } else if(g_idx == 12) {
                c->s_max_x = stroul(buf);
            } else if(g_idx == 13) {
                c->s_max_y = strtoul(buf);
            } else {
                strncpy(c->pass, buff, 16);
            }

            g_idx++;
            buf_idx = 0;
            continue;
        }

        buf[buf_idx] = ch;
        buf_idx++;
    }

    return 1;
}


int main(int argc, char **argv) {

    //---------- Detection thread ----------
    MotionConfig motion_config;
    motion_config.blur_size = 5;
    motion_config.motion_thresh = 50;

    MotionDetector md;
    int err = md_init(&md, &motion_config);
    //printf("init err = %d\n", err);

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
   

    if(load_config(&user_config) < 0) {
        perror("ERR: ");
        exit(1);
    }
    /*
    user_config.mode = MANUAL;
    user_config.sys = 1;
    user_config.move_x = 0;
    user_config.move_y = 0;
    */

    //servo init
    struct servo_controller sc;
	printf("before\n");
    servo_controller_init(&sc);
    printf("after\n");
    
    //packet buffer
    Packet packet;
    size_t size = 0;
    
    struct timeval time_struct;
    gettimeofday(&time_struct, NULL);
    long long last_time = (time_struct.tv_sec * 1e3 + time_struct.tv_usec / 1e3);

    int last_x = 0;
    int last_y = 0;

    // Set timer for calling the servo update function every 20 ms
    //gpioSetTimerFuncEx(0, 20, update_servos, &sc);

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

            //printf("MANUAL: (%hd, %hd)\n", user_config.move_x, user_config.move_y);
            
            //TODO:these flags should seriously be locked
            //turn off detection
           
            md_disable_detection(&md);

            //code to move servo according to move_x and move_y (relative value to center of screen)
            //code to sleep appropreate amount of time until servo completed rotation
	        //float pX = user_config.move_x / user_config.motion_config.max_x;			
	        //int move = (int) (pX * ());

		
	    	//if theres no movement skip
		    if(user_config.move_x == 0 && user_config.move_y == 0) {
			    continue;
		    }

		servo_controller_turn(&sc, -1 * (int)(user_config.move_x ),(int)( user_config.move_y));
		//MoveServos(user_config, sc, step_range);	
		//turn on detection
		

            md_enable_detection(&md);

		user_config.move_x = 0;
		user_config.move_y = 0;

            continue;
        }

        if(user_config.mode == AUTO) {
        
            //printf("AUTO: (%hd, %hd)\n", user_config.move_x, user_config.move_y);

            md_enable_detection(&md);
            gettimeofday(&time_struct, NULL);
            long long curr_time = (time_struct.tv_sec * 1e3 + time_struct.tv_usec / 1e3);
            //if we seconds passed
            if(curr_time - last_time >= 200 && md.center_count > 0) {
		user_config.move_x = (md.total_center_x / md.center_count) - (md.config->max_x / 2);
		user_config.move_y = (md.total_center_y / md.center_count) - (md.config->max_x / 2);
		//printf("(%d,%d)\n",user_config.move_x, user_config.move_y);
                md_disable_detection(&md); 
                process_detected_input(&sc, &md, &user_config);
		usleep(100 * 1000);
	        //printf("ending delay \n");
		    
                //turn on detection
                md_enable_detection(&md);

                //should mutex lock happen here? do we care a few frames of inaccuracy?
                md.total_center_x = 0;
                md.total_center_y = 0;
                md.center_count = 0;
		
		gettimeofday(&time_struct, NULL);
                curr_time = (time_struct.tv_sec * 1e3 + time_struct.tv_usec / 1e3);
                last_time = curr_time;
            } 
            
            continue;
        }
    }

    //---------- control thread finished ----------
    //system shutdown kill all other threads

    connection.state = STOP;
    md_stop_thread(&md);
    pthread_join(network_thread, NULL);  
    pthread_join(detection_thread, NULL);  
}
