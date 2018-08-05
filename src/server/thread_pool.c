
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "server/server.h"
#include "server/thread_pool.h"
#include "thread_pool.h"

#ifndef INVALID
#define INVALID -1
#endif

net_element *dequeue(_thread_pool *th_pool) {

    _queue_node *q_node = th_pool->queue_node;

    net_element *net_info;

    while (q_node->qsize == 0) {
        pthread_cond_wait(&(th_pool->notify), &(th_pool->lock));
    }

    net_info = q_node->queue[q_node->front].arg;

    --q_node->qsize;
    ++q_node->front;

    if (q_node->front == q_node->qcapacity)
        q_node->front = 0;

    return net_info;
}

void *th_pool_worker(void *ptr) {

    _thread_pool *th_pool = (_thread_pool *) ptr;

    while (1) {
        if (th_pool->queue_node->qsize == 0) {
            usleep(SLEEP_INTERVAL_MIN);
            continue;
        }
        pthread_mutex_lock(&th_pool->lock);
        net_element *net_info;
        net_info = dequeue(th_pool);
        gettimeofday(&net_info->dequeue_time, NULL);
        pthread_mutex_unlock(&(th_pool->lock));
        net_info->handle_buf(net_info);
        // Here net_info should be freed

        usleep(SLEEP_INTERVAL_MIN);
    }

}

int enqueue(_thread_pool *th_pool, void *job) {


    _queue_node *qnode = th_pool->queue_node;

    if (qnode->qsize == qnode->qcapacity) {

        return INVALID;
    }
    else {

        ++qnode->qsize;
        qnode->rear += 1;
        if (qnode->rear == qnode->qcapacity)
            qnode->rear = 0;
        qnode->queue[qnode->rear].arg = job;
        gettimeofday(&((net_element*)qnode->queue[qnode->rear].arg)->enqueue_time, NULL);

        return 0;
    }

}

_queue_node *creaet_queue(int q_size) {

    _queue_node *qnode;

    if ((qnode = (_queue_node *) malloc(sizeof(_queue_node))) == NULL)
        return NULL;

    if ((qnode->queue = (_queue *) malloc(sizeof(_queue) * q_size)) == NULL)
        return NULL;

    qnode->front = 0;
    qnode->rear = -1;
    qnode->qsize = 0;
    qnode->qcapacity = q_size;

    return qnode;

}


_thread_pool *create_thread_pool(int th_number, int queue_size) {

    if (th_number <= 0 || queue_size <= 0)
        return NULL;

    int indx;
    _thread_pool *th_info;
    th_info = (_thread_pool *) malloc(sizeof(_thread_pool));
    if (th_info == NULL)
        return NULL;

    th_info->thread_count = 0;

    if ((th_info->threads = (pthread_t *) malloc(sizeof(pthread_t) * th_number)) == NULL)
        goto error;

    th_info->queue_node = creaet_queue(queue_size);


    if ((pthread_mutex_init(&(th_info->lock), NULL) != 0) || (pthread_cond_init(&(th_info->notify), NULL) != 0))
        goto error;

    for (indx = 0; indx < th_number; indx++) {
        pthread_create(&(th_info->threads[indx]), NULL, &th_pool_worker, (void *) th_info);
        ++th_info->thread_count;

    }

    return th_info;

    error:
    if (th_info)
        free(th_info);
    return NULL;
}

int append_job(_thread_pool *th_pool, void *job, void *ptr) {

    if (th_pool == NULL)
        return INVALID;
    _queue_node *qnode = th_pool->queue_node;
    if (qnode->qsize == qnode->qcapacity) {

        return INVALID;
    }

    if (pthread_mutex_lock(&th_pool->lock) != 0)
        return INVALID;

    if (enqueue(th_pool, job) == INVALID) {

        pthread_mutex_unlock(&th_pool->lock);
        return INVALID;
    }

    pthread_cond_signal(&(th_pool->notify));
    if (pthread_mutex_unlock(&th_pool->lock) != 0)
        return INVALID;

    return 0;
}