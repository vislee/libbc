/*
 *file name : rz_thread_pool.c
 *des : 
 *date : 2015-06-14
 *author : liwq (286570361)
 *notes :
 *       2013-10-21   liwq      create files
 */

/**********************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
 
#include "rz_thread_pool.h"


static void* 
thread_routine(void *arg)
{
    rz_thread_pool_t       *tpool = (rz_thread_pool_t *)arg;
    rz_thread_pool_work_t  *work;

    while(1) {
        pthread_mutex_lock(&tpool->queue_lock);

        while(!tpool->queue_head && !tpool->shutdown) {
            #ifdef __DEBUG__
              printf("[%lu]i sleep\n", pthread_self());
            #endif
            pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
        }

        if (tpool->shutdown) {
            #ifdef __DEBUG__
              printf("[%lu]i quit\n", pthread_self());
            #endif
            pthread_mutex_unlock(&tpool->queue_lock);
            pthread_exit(NULL);
        }

        #ifdef __DEBUG__
          printf("[%lu]i awake\n", pthread_self ());
        #endif
        work = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        
        if (!tpool->queue_head) {
          pthread_cond_signal(&tpool->queue_empty);
        }

        pthread_mutex_unlock(&tpool->queue_lock);

        work->routine(work->arg);
        free(work->arg);
        free(work);
    }

    return NULL;   
}



int
rz_thread_pool_create(rz_thread_pool_t **prtpt, int max_thr_num)
{
    int i;
    rz_thread_pool_t *rtpt = NULL;
    rtpt = calloc(1, sizeof(rz_thread_pool_t));
    if (NULL == rtpt) {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(1);
    }


    rtpt->max_thr_num = max_thr_num;
    rtpt->shutdown = 0;
    rtpt->list_close = 0;
    rtpt->queue_head = NULL;

    if (pthread_mutex_init(&rtpt->queue_lock, NULL) != 0) {
        printf("%s: pthread_mutex_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        exit(1);
    }

    if (pthread_cond_init(&rtpt->queue_ready, NULL) != 0) {
        printf("%s: pthread_cond_init failed, errno:%d, error:%s\n", 
            __FUNCTION__, errno, strerror(errno));
        exit(1);
    }

    if (pthread_cond_init(&rtpt->queue_empty, NULL) !=0 ) {
        printf("%s: pthread_cond_init2 failed, errno:%d, error:%s\n", 
            __FUNCTION__, errno, strerror(errno));
        exit(1);
    }


    rtpt->thr_id = calloc(max_thr_num, sizeof(pthread_t));
    if (NULL == rtpt->thr_id) {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(1);
    }

    for (i = 0; i < max_thr_num; ++i) {
        if (pthread_create(&rtpt->thr_id[i], NULL, thread_routine, (void*)rtpt) != 0){
            printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, 
                errno, strerror(errno));
            exit(1);
        }

    }
    *prtpt = rtpt;
    return 0;
}



void
rz_thread_pool_destroy(rz_thread_pool_t *rtpt, int flag)
{
    int i;
    rz_thread_pool_work_t *member;

    if (rtpt->list_close) {
        return;
    }

    pthread_mutex_lock(&rtpt->queue_lock);
    rtpt->list_close =1;

    if (flag) {
        if (rtpt->queue_head) {
            pthread_cond_wait(&rtpt->queue_empty, &rtpt->queue_lock);
        }
    }
    rtpt->shutdown = 1;


    pthread_cond_broadcast(&rtpt->queue_ready);
    pthread_mutex_unlock(&rtpt->queue_lock);
    for (i = 0; i < rtpt->max_thr_num; ++i) {
        pthread_join(rtpt->thr_id[i], NULL);
    }
    free(rtpt->thr_id);

    while(rtpt->queue_head) {
        member = rtpt->queue_head;
        rtpt->queue_head = rtpt->queue_head->next;
        free(member->arg);
        free(member);
    }

    pthread_mutex_destroy(&rtpt->queue_lock);    
    pthread_cond_destroy(&rtpt->queue_ready);

    free(rtpt);
}



int
rz_thread_pool_add_work(rz_thread_pool_t *rtpt, void*(*routine)(void*), void *arg, int len)
{
    rz_thread_pool_work_t *work, *member;

    if (NULL == routine){
        printf("%s:Invalid argument\n", __FUNCTION__);
        return -1;
    }

    work = malloc(sizeof(rz_thread_pool_work_t));
    if (NULL == work) {
        printf("%s:malloc failed\n", __FUNCTION__);
        return -1;
    }

    work->routine = routine;
    //work->arg = arg;
    work->arg = (void*)malloc(len);
    if (NULL == work->arg) {
        free(work)
        return 0;
    }
    //memset(work->arg, 0x00, 100);
    memcpy(work->arg, arg, len);
    work->next = NULL;
 
    pthread_mutex_lock(&tpool->queue_lock);
    if (tpool->list_close) {
        pthread_mutex_unlock(&tpool->queue_lock);
        return 0;
    }

    member = tpool->queue_head;
    if (NULL == member) {
        tpool->queue_head = work;
    } else {
        while(member->next) {
            member = member->next;
        }
        member->next = work;
    }

    pthread_cond_signal(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);
 
    return 0;    
}
