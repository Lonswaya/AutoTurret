#include "Networking.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 14000           //change this
#define TIMEOUT 30          //seconds

int net_init(Connection *c) {

    c->queue = (PacketQueue *) malloc(sizeof(PacketQueue));
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

    c->server_socket = 0;
    c->socket = 0;
    c->state = DED;
    return 1;
}

/* 
 * Private function used to set up a socket to be listened on
 */
int _net_init_socket(Connection *c) {

    //newly init socket is 0
    if(c->server_socket != 0) {
        close(c->server_socket);
    }

    struct protoent *proto = getprotobyname("tcp");
    if(proto == NULL) {
        //can't get protocol?
        return -1;
    }

    c->server_socket = socket(AF_INET, SOCK_STREAM, proto->p_proto);
    if(c->socket < 0) {
        //can't create socket
        return -2;
    }

    int yes = 1;
    if(setsockopt(c->server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        //cant be reused??
        return -3;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if(bind(c->server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        //can't bind
        return -4;
    }

    if(listen(c->server_socket, 10) < 0) {
        //cant listen
        return -5;
    }

    c->state = INIT;
    return 1;
}

int net_packetq_size(Connection *c, size_t *size) {

    if(pthread_mutex_lock(&(c->queue->lock)) != 0) {
        //can't lock ????
        return -1;
    }

    *size = c->queue->size;

    pthread_mutex_unlock(&(c->queue->lock));
    return 1;
}

int net_get_packet(Connection *c, Packet *p) {
    
    if(pthread_mutex_lock(&(c->queue->lock)) != 0) {
        //can't lock ????
        return -1;
    }

    if(c->queue->size == 0 || c->queue->head == NULL) {
        //queue empty

        pthread_mutex_unlock(&(c->queue->lock));
        return -2;
    }
    
    PacketQEnt *head = c->queue->head;
    memcpy(p, head, sizeof(Packet));
    
    c->queue->head = head->next;

    //if tail happens to point to this too (only packet in queue) fix tail pointer to null
    if(c->queue->tail == head) {
        c->queue->tail = NULL;
    }

    head->next = NULL;
    free(head); 
    c->queue->size--;
    
    //should check err on this too but waht do if erred?
    pthread_mutex_unlock(&(c->queue->lock));

    return 1;
}

/* 
 * Private function used for the networking loop to recv packet
 * then put it in the queue
 */
int _net_put_packet(Connection *c, Packet *p) {
    
    //could have a max queue size check here
    
    if(p == NULL) {
        //should neve happen but just in case
        return -1;
    }

    PacketQEnt *new_p = (PacketQEnt *) malloc(sizeof(PacketQEnt));
    if(new_p == NULL) {
        //cant malloc
        return -2;
    }

    memcpy(new_p, p, sizeof(Packet));
    new_p->next = NULL;

    if(pthread_mutex_lock(&(c->queue->lock)) != 0) {
        return -3;
    }

    PacketQEnt *tail = c->queue->tail;

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

void *net_loop(void *arg) {

    Connection *c = (Connection *) arg;
    struct sockaddr client_addr;
    socklen_t client_addr_len;
    
    while(c->state != STOP) { 
        
        if(c->state == DED) {
            //if socket is dead then we reinit the socket
            if(_net_init_socket(c) < 0) {
                continue;
            }
            
            printf("Ready to accept\n");
            c->socket = accept(c->server_socket, &client_addr, &client_addr_len);

            if(c->socket == -1) {

                //err on accept?
                c->state = DED;
                c->server_socket = 0;
                c->socket = 0;
                continue;
            }
        
            struct timeval timeout;
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = 0; 
            
            if(setsockopt(c->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout) < 0)) {

                //recv timeout failed.. but we can still go
                
            }

            c->state = READY;
        }

        if(c->state == BUZY) {
            continue;
        }

        if(c->state == READY) {
                
            Packet packet;
            size_t bytes;

            bytes = recv(c->socket, &packet, sizeof(packet), 0);            
            if(bytes <= 0) {
                //errors here usually means it needs a restart
                //0 means connection closed
                //< 0 is error timeout also falls here so becareful
                //as of now we treat them all as fatal error that needs a restart
                c->state = DED;
                continue;
            }            

            //printf("\nRecv Packet with type: %d\n", packet.type);
            _net_put_packet(c, &packet);
        }
    }

    //thread stopped clean up
    close(c->socket);
    close(c->server_socket);

    //should probably free queues too.
}
