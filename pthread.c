#include "pthread.h"

/*
    routine:任务调配函数(线程函数)，让所有的线程都要执行此函数，此函数会不断的从任务队列中取任务去执行
    如何取任务 -->arg 把线程池传递进来
*/	
void *routine(void *arg)
{
    pthread_pool *pool = (pthread_pool *)arg;
    while(1)
    {
        pthread_mutex_lock(&pool->mutex);
        while(0 == pool->cur_waiting_task && 1 == pool->shutdown)
        {
            pthread_cond_wait(&pool->cond,&pool->mutex);
        }
        if(0 == pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        //去取任务-->把第一个任务结点从链表中拆除
        Task *task = NULL;
        if(pool->tasklist)
        {
            task = pool->tasklist;
            //pool->tasklist = pool->tasklist->next;
            pool->tasklist = (Task*) pool->tasklist->next;
            pool->cur_waiting_task--;
        }
        else
        {
            continue;
        }
        //V操作
        pthread_mutex_unlock(&pool->mutex);
        //printf("src: %s\n",((struct cp_file *)(task->arg))->src);
        //执行任务
        (task->do_task)(task->arg);
        //释放结点
        task->next = NULL;
        free(task);
        task = NULL;
    }
    return NULL;
}
/*
		init_pool:线程池初始化函数
		指定要初始化的线程池，线程池中有thread_num个线程
        @pool:指针，指向要初始化的线程池
        @thread_num:线程池一开始线程的个数
        @返回值： 
            成功返回0，失败返回-1
*/
int init_pool(pthread_pool *pool,unsigned int thread_num)
{
    //初始化线程池结构体
    //初始化线程池互斥锁
    //初始化条件变量
    //创建thread_num个线程，同时tids这个指针去记录所有线程的ID
    //......
    pthread_mutex_t mutex;
    pool->mutex = mutex;
    pthread_mutex_init(&pool->mutex,NULL);
    pthread_cond_t cond;
    pool->cond = cond;
    pthread_cond_init(&pool->cond,NULL);
    pool->tasklist = NULL;
    pool->tids = malloc(sizeof(pthread_t) * thread_num);
    pthread_t *tid = pool->tids;
    pool->active_pthreads = thread_num;
    pool->cur_waiting_task = 0;
    pool->shutdown = 1;
    int i = 1;
    while(i <= thread_num)
    {
        if(-1 == pthread_create(tid, NULL, (void *)routine, (void *)pool))
        {
            perror("pthread_create failed!");
            return -1;
        } 
        long int x=(long int)tid;
        printf("#tid :%ld\n",x);
        tid++;
        i++;
    }
    return 0;
}
/*
    销毁线程池：销毁线程池的时候注意所有的任务一定都是完成了(所有的线程都被回收了)
    
*/
int destroy_pool(pthread_pool *pool)
{
    //释放所有空间，等待任务执行完毕 -->等任务队列当中结点数为0
    //shutdown = 0;
    //唤醒所有线程
    //pthread_join
    while(pool->cur_waiting_task)
    {
        //printf("pool->cur_waiting_task: %d\n", pool->cur_waiting_task);
    }
    pool->shutdown = 0;
    pthread_cond_broadcast(&pool->cond);
    pthread_t *tid = pool->tids;
    int i = 1;
    while(i <= pool->active_pthreads)
    {
        pthread_join(*tid,NULL);
        tid++;
        i++;
    }
    if(pthread_mutex_destroy(&pool->mutex))
    {
        printf("pthread_mutex_destroy failed!\n");
        return -1;
    }
    if(pthread_cond_destroy(&pool->cond))
    {
        printf("pthread_cond_destroy failed!\n");
        return -1;
    }
    free(pool->tids);
    pool->tids = NULL;
    pool->active_pthreads = 0;
    return 0;
}
/*
    add_task:给任务队列增加任务，fun_task和arg两个成员封装到struct task结点中，把该结点加入到任务队列中(尾插法)
    @pool：要添加任务的线程池
    @fun_task：指向任务结点中的任务函数
    @arg：你要执行的任务的参数
    @返回值：成功返回0，失败返回-1.
*/
int add_task(pthread_pool *pool,void (*fun_task)(void *arg),void *arg)//(void *arg)
{
    printf("add_task\n");
    //需要把第二个参数和第三个参数封装成struct task的形式
    //P操作
    //把该节点加入到任务队列中
    //唤醒所有线程
    //V操作
    Task *task = malloc(sizeof(Task));
    task->do_task = fun_task;
    task->arg = arg;
    task->next = NULL;
    pthread_mutex_lock(&pool->mutex);
    Task *tasklist = pool->tasklist;
    if(!tasklist)
    {
        pool->tasklist = task;
    }
    else
    {
        while(tasklist->next)
        {
            tasklist = (Task*) tasklist->next;
        }
        tasklist->next = (Task*) task;
    }
    pool->cur_waiting_task++;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    return 0;
}
