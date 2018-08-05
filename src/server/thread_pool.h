#ifndef GEODNS_THREAD_POOL_H
#define GEODNS_THREAD_POOL_H

#include <pthread.h>

#include "server/server.h"


typedef struct{
    void *arg;
}_queue;

typedef struct {
    int rear;
    int front;
    int qsize;
    int qcapacity;
    _queue *queue;
}_queue_node;



typedef struct {

    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    int thread_count;
    _queue_node * queue_node;

}_thread_pool;


_thread_pool *create_thread_pool(int, int);
int append_job(_thread_pool *th_pool, void *, void*);



#endif // GEODNS_THREAD_POOL_H
