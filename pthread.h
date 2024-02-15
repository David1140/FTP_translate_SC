#ifndef __PTHREAD_H__
#define __PTHREAD_H__
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct task{
    //每一个任务结点保存一个任务的所有信息，所谓的任务就是把一个文件拷贝到另外一个文件里
    //这个任务怎么保存
    //任务的实现利用函数实现的,那么我们要保存这个任务的话，只需要保存任务函数就可以了
    //还需要保存这个函数执行所需要的参数
    //所以这里需要定义一个函数指针
    int client_sock;
    off_t start_offset;
    off_t end_offset;
    char *file_name;
    void (*do_task)(void *arg);
    void *arg;
    struct task *next;//指向下一个任务
} Task;

// void* handle_task(void* arg) {
//     Task* task = (Task*)arg;
//     // TODO: 实现文件传输逻辑，根据start_offset和end_offset来进行断点传输
//     close(task->client_sock);
//     free(task->file_name);
//     free(task);
//     return NULL;
// }

// void add_task_to_thread_pool(Task* task) {
//     // TODO: 将任务添加到线程池中
//     pthread_t tid;
//     pthread_create(&tid, NULL, handle_task, task);
//     pthread_detach(tid);
// }
typedef struct pthread_pool
{
    //线程池的实现按照项目的不同有所不同，但是大概的成员如下：
    //"任务队列"是一种贡献资源，所以需要互斥访问
    //需要线程互斥锁
    pthread_mutex_t mutex;
    //同时当我们的任务队列中没有任务的时候，线程池内的线程应该要休眠
    //需要条件变量
    pthread_cond_t cond;
    //"任务队列"(链表)，指向第一个需要执行的任务
    //所有的线程都需要从这个链表中取任务
    Task *tasklist;
    //指向线程ID的数组，用来保存线程池中所有线程的ID
    pthread_t *tids;
    //线程池中正在执行任务的线程数量
    unsigned int active_pthreads;
    //线程池"任务队列"中当前任务的数量
    unsigned int cur_waiting_task;
    //表示是否退出程序
    int shutdown;//0表示退出 1表示不退出
}pthread_pool;
int init_pool(pthread_pool *pool,unsigned int thread_num);
int destroy_pool(pthread_pool *pool);
int add_task(pthread_pool *pool,void (*fun_task)(void *arg),void *arg);
//void* my_tcp_translate_task(void* arg);
#endif