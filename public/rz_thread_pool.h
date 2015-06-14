/*
 *file name : rz_thread_pool.h
 *des : 
 *date : 2015-06-14
 *author : liwq (286570361)
 *notes :
 *       2013-10-21   liwq      create files
 *       2015-06-14   liwq      modify files
 */

/**********************************************************/
#ifndef __RZ_THREAD_POOL_H__
#define __RZ_THREAD_POOL_H__
 
#include <pthread.h>
 

typedef struct rz_thread_pool_work_s  rz_thread_pool_work_t;

struct rz_thread_pool_work_s {
    void*                     (*routine)(void*);
    void                       *arg;
    rz_thread_pool_work_t      *next;
};


typedef struct rz_thread_pool_s  rz_thread_pool_t;
struct rz_thread_pool_s {
    int                       shutdown;
    int                       list_close;
    int                       max_thr_num;
    pthread_t                *thr_id;
    rz_thread_pool_work_t    *queue_head;
    pthread_mutex_t           queue_lock;
    pthread_cond_t            queue_ready;
    pthread_cond_t            queue_empty;
};


int rz_thread_pool_create(rz_thread_pool_t **prtpt, int max_thr_num);


void rz_thread_pool_destroy(rz_thread_pool_t *rtpt, int flag);
 

int rz_thread_pool_add_work(rz_thread_pool_t *rtpt, void*(*routine)(void*), void *arg, int len);

#endif  /*__RZ_THREAD_POOL_H__*/
