#include "Networking.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int net_init(Connection *c, int port) {

    c->queue = malloc(sizeof(PacketQueue));
    if(c->queue == NULL) {
        //couldn't allocate
        return -1;
    }    

    c->queue->head = c->queue->tail = NULL;
    c->queue->size = 0;

    if(pthread_mutex_init(&(c->queue->lock), NULL) != 0) {
        //couldn't init lock
        return -2;
    }

    struct protoent *proto = getprotobyname("tcp");
    if(proto == NULL) {
        //can't get protocol?
        return -3;
    }

    c->socket = socket(AF_INET, SOCK_STREAM, proto->p_proto);
    if(c->socket < 0) {
        //can't create socket
        return -4;
    }

    c->state = INIT;
    return 1;
}

int net_get_packet(Connection *c, Packet *p) {
    
    if(pthread_mutex_lock(&(c->queue->lock) != 0)) {
        //can't lock ????
        return -1;
    }

    if(c->queue->size == 0 || c->queue->head == NULL) {
        //queue empty

        pthread_mutex_unlock(&(c->queue->lock));
        return -2;
    }
    
    Packet *head = c->queue->head;
    memcpy(p, head, sizeof(Packet));
    
    c->queue->head = head->next;

    //if tail happens to point to this too (only packet in queue) fix tail pointer to null
    if(c->queue->tail == head) {
        c->queue->tail = NULL;
    }

    head->next = NULL;
    free(c->queue->head); 
    c->queue->size--;
    
    //should check err on this too but waht do if erred?
    pthread_mutex_unlock(&(c->queue->lock));

    return 1;
}

int _net_put_packet(Connection *c Packet *p) {
    
    //could have a max queue size check here
    
    if(p == NULL) {
        //should neve happen but just in case
        return -1;
    }

    Packet *new_p = malloc(sizeof(Packet));
    if(new_p == NULL) {
        //cant malloc
        return -2;
    }

    memcpy(new_p, p, sizeof(Packet));
    
    if(pthread_mutex_lock(&(c->queue->lock)) != 0) {
        return -3;
    }

    Packet *tail = c->queue->tail;

    if(tail == NULL) {
        c->queue->head = c->queue->tail = new_p;
    } else {
        tail->next = new_p;
        c->queue->tail = new_p;
    }

    c->queue->size++;

    pthread_mutex_unlock(&(c->queue->lock));

    return 1;
}

void network_loop(void *arg) {

}
