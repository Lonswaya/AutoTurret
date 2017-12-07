#ifndef NETWORKING_H_
#define NETWORKING_H_

#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdint.h>

#include "../include/UserConfig.h"

//Packet type
// 0 - mode control
// 1 - manual direction data
// 2 - system control

typedef struct net_packet {
    uint32_t type;       //see design doc on this
    uint32_t data;

} Packet;

typedef struct net_packet_q_ent PacketQEnt;

struct net_packet_q_ent{
    Packet p;
    PacketQEnt *next;
};

typedef struct net_packet_q {
    size_t size;    //good to have just in case we want to limit the queue
    PacketQEnt *head;   //read from head
    PacketQEnt *tail;   //append to tail
    pthread_mutex_t lock;

} PacketQueue;

typedef enum net_sock_state {
    INIT,   //when socket just created listning for connection 
    READY,  //when socket can be used to recv
    ERR,    //any error happened, reset socket
    BUZY,   //socket is doing things cannot be used on other things
    DED,    //socket needs to be initialized
    STOP,   //flag to tell the networking thread to stop 
    AUTH    //got a connection waiting to be authed
} SOCK_STATE;

//a connection struct used to abstract everything needed with networking
typedef struct net_conn {
    PacketQueue *queue;     //queue for the network
    int socket;             //the socket that will be used to communicate with client   
    int server_socket;      //the socket used to bind then listen
    SOCK_STATE state;       //state the socket is in
    UserConfig* config;
} Connection;

/* Initiliaze a connection
 * create socket
 * initialize/allocate queue
 * init lock
 */
int net_init(Connection *c, UserConfig* config);

/*
 * Gets a packet from the queue
 * the packet will be filled into p
 * caller responsible allocating p
 */
int net_get_packet(Connection *c, Packet *p);

/*
 * Checks the size of queue
 * Needs to be a function because we want to mutex lock this 
 * to prevent possible(?) race condition 
 */
int net_packetq_size(Connection *c, size_t *size);

/*
 * Networking loop arg will be a Connection pointer
 */
void *net_loop(void *arg);



#endif
