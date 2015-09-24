#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

struct buffer {
    int size;
    int start;
    int count; // number of elements in buffer
    void **element;
};
 
typedef struct buffer buffer_t;

void buffer_init(buffer_t*, int);
int full(buffer_t*);
int empty(buffer_t*);
void buffer_push(buffer_t*, void*);
void * buffer_popqueue(buffer_t*);
void * buffer_popstack(buffer_t*);

#endif