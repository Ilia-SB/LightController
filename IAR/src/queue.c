#include "queue.h"

void buffer_init(buffer_t *buffer, int size) {
    buffer->size = size;
    buffer->start = 0;
    buffer->count = 0;
    buffer->element = malloc(sizeof(buffer->element)*size);
    /* allocated array of void pointers. Same as below */
    //buffer->element = malloc(sizeof(void *) * size);
     
}
 
int full(buffer_t *buffer) {
    if (buffer->count == buffer->size) { 
        return 1;
    } else {
        return 0;
    }
}
 
int empty(buffer_t *buffer) {
    if (buffer->count == 0) {
        return 1;
    } else {
        return 0;
    }
}
     
void buffer_push(buffer_t *buffer, void *data) {
    int index;
    if (full(buffer)) {
    } else {
        index = buffer->start + buffer->count++;
        if (index >= buffer->size) {
            index = 0;
        }
        buffer->element[index] = data;
    }
}
 
 
void * buffer_popqueue(buffer_t *buffer) {
    void * element;
    if (empty(buffer)) {
        //printf("Buffer underflow\n");
        return NULL;
    } else {
       /* FIFO implementation */
       element = buffer->element[buffer->start];
       buffer->start++;
       buffer->count--;
       if (buffer->start == buffer->size) {
           buffer->start = 0;
       }
        
       return element;
    }
}
 
void * buffer_popstack(buffer_t *buffer) {
    int index;
    if (empty(buffer)) {
        return NULL;
    } else {
        /* LIFO implementation */
        index = buffer->start + buffer->count - 1;
        if (index >= buffer->size) {
           index = buffer->count - buffer->size - 1;
        }      
        buffer->count--;
        return buffer->element[index];
    }
}