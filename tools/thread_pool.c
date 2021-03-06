#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
 
#include "thread_pool.h"
#include "debug.h"
 
static tpool_t *tpool = NULL;

int MAX_TPOOL_THREAD_NUM = 300;

  
/* 工作者线程函数, 从任务链表中取出任务并执行 */
void* 
thread_routine(void *arg)
{
    tpool_work_t *work;

    SQL_STRUCT current_sqls;
    SQL_STRUCT* current_sql;
    
    int num;
    num = *((int*)arg);

    current_sql = &current_sqls;

    current_sqls.logFd = init_thread_log(num);

    memset(current_sqls.sqlCmd,0,sizeof(current_sqls.sqlCmd));

    
    //此处使用长连接连接数据库
    //存在问题:某些网络未按照TCP/IP中的老化时间，这里又没有设置定时心跳，可能导致操作数据库时连接超时。
    connect_new_SQL(&(current_sqls.my_connection));
    
    while(1) {
        /* 如果线程池没有被销毁且没有任务要执行，则等待 */
        pthread_mutex_lock(&tpool->queue_lock);
        while(!tpool->queue_head && !tpool->shutdown) {
            pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
        }
        if (tpool->shutdown) {
            pthread_mutex_unlock(&tpool->queue_lock);
            pthread_exit(NULL);
        }
        work = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        pthread_mutex_unlock(&tpool->queue_lock);
 
        work->routine(work->arg,&current_sqls);
        FREE(work);
    }
    
    return NULL;   
}
 
/*
 * 创建线程池 
 */
int
tpool_create(int max_thr_num)
{
    int i;
    int* threadIndex;

    threadIndex = MALLOC(max_thr_num*sizeof(int));

    for(i=0;i<max_thr_num;i++)
    {
        threadIndex[i] = i;
    }
 
    tpool = (tpool_t *)calloc(1, sizeof(tpool_t));
    if (!tpool) {
        DEBUGPRINT;
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(1);
    }
    
    /* 初始化 */
    tpool->max_thr_num = max_thr_num;
    tpool->shutdown = 0;
    tpool->queue_head = NULL;
    
    if (pthread_mutex_init(&tpool->queue_lock, NULL) !=0) {
        printf("%s: pthread_mutex_init failed, errno:%d, error:%s\n",
            __FUNCTION__, errno, strerror(errno));
        exit(1);
    }
    if (pthread_cond_init(&tpool->queue_ready, NULL) !=0 ) {
        printf("%s: pthread_cond_init failed, errno:%d, error:%s\n", 
            __FUNCTION__, errno, strerror(errno));
        exit(1);
    }
    
    /* 创建工作者线程 */
    tpool->thr_id = (pthread_t *)calloc(max_thr_num, sizeof(pthread_t));
    if (!tpool->thr_id) {
        printf("%s: calloc failed\n", __FUNCTION__);
        exit(1);
    }
    for (i = 0; i < max_thr_num; ++i) {
        if (pthread_create(&tpool->thr_id[i], NULL, thread_routine, (void*)&threadIndex[i]) != 0){
            printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, 
                errno, strerror(errno));
            exit(1);
        }
        
    }    
 
    return 0;
}
 
/* 销毁线程池 */
void
tpool_destroy()
{
    int i;
    tpool_work_t *member;
 
    if (tpool->shutdown) {
        return;
    }
    tpool->shutdown = 1;
 
    /* 通知所有正在等待的线程 */
    pthread_mutex_lock(&tpool->queue_lock);
    pthread_cond_broadcast(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);
    for (i = 0; i < tpool->max_thr_num; ++i) {
        pthread_join(tpool->thr_id[i], NULL);
    }
    FREE(tpool->thr_id);
 
    while(tpool->queue_head) {
        member = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        FREE(member);
    }
 
    pthread_mutex_destroy(&tpool->queue_lock);    
    pthread_cond_destroy(&tpool->queue_ready);
 
    FREE(tpool);    
}
 
/* 向线程池添加任务 */
int
tpool_add_work(void*(*routine)(void*,SQL_STRUCT*), void *arg)
{
    tpool_work_t *work, *member;
    
    if (!routine){
        printf("%s:Invalid argument\n", __FUNCTION__);
        return -1;
    }
    
    work = (tpool_work_t *)MALLOC(sizeof(tpool_work_t));
    if (!work) {
        printf("%s:malloc failed\n", __FUNCTION__);
        return -1;
    }
    work->routine = routine;
    work->arg = arg;
    work->next = NULL;
 
    pthread_mutex_lock(&tpool->queue_lock);    
    member = tpool->queue_head;
    if (!member) {
        tpool->queue_head = work;
    } else {
        while(member->next) {
            member = member->next;
        }
        member->next = work;
    }
    /* 通知工作者线程，有新任务添加 */
    pthread_cond_signal(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);
 
    return 0;    
}