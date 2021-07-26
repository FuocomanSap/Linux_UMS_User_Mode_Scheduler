#include "Headers/ums_user_library.h"
#include <stdio.h>
#include <pthread.h>

#define proc_test 0
#define NUM_THREAD 8
#define NUM_UMS 2
#define NUM_WORKERS 10000 //the total number will be double this, since we add 2 batches of NUM_WORKERS workers
#define YIELD_TEST 0
typedef struct args
{
    int fd;
    int i;
    ums_pid_t comp_list_id;
    int padre;
} args;

int a = 0; //executed worker
int b = 2; //added worker
pthread_t thread[NUM_THREAD];
args p_arg[NUM_THREAD];
int fd[NUM_UMS];
pthread_mutex_t mutex;

int entry_point(void *arg)
{

    args *param = (args *)arg;
    int fd = param->fd;
    int ret = 0;
    completion_list_data *list_data;

    list_data = dequeue_ums_completion_list_items(fd);

    if (list_data == NULL)
    {
        printf("ENTRYPOINT error in deque\n");
        return -1;
    }

    if (list_data->head == NULL)
    {
        printf("completion list VUOTA\n");
        return -1;
    }

    ums_worker_thread_data *worker = list_data->head;

    while (worker != NULL)
    {

        ret = execute_ums_thread(fd, worker->pid, list_data);

        if (ret != -EACCES && ret != -EBADF && ret != -EFBIG)
        {

            break;
        }
        if (ret == -EFBIG)
        {
            printf("ERROR: cant alloc\n");

            break;
        }
        worker = worker->next;
    }

    return 0;
}

int work(void *arg)
{
    int err;
    args *param = (args *)arg;

    if (YIELD_TEST)
    {

        err = ums_thread_yield(param->fd);
        if (err < 0)
        {
            printf("YIELD ERROR, err:%d\n", err);
            return err;
        }
    }

    if (proc_test)
        sleep(60);

    int first = 1;
    int second = 1;

    int N = 5000000;
    for (int i = 2; i < N; i++)
    {
        int third = first + second;
        first = second;
        second = third;
        //printf("%d\n", third);
    }

    pthread_mutex_lock(&mutex);
    a++;
    pthread_mutex_unlock(&mutex);

    return 0;
}

void *thread_func(void *arg)
{
    args *param = (args *)arg;
    int ret;
    printf("sto per chiamare enter ums scheduling mode\n");
    printf("param->padre:%d\n", param->padre);
    ret = enter_ums_scheduling_mode(param->fd, &entry_point, param, param->comp_list_id);
}

int main()

{
    pthread_mutex_init(&mutex, NULL);
    completion_list_data *comp_list;
    int i;
    int workers_added = 0;
    printf("apriro'\n");
    for (i = 0; i < NUM_UMS; i++)
    {
        fd[i] = ums_init();
        if (fd[i] < 0)
            return -1;
        printf("opened:%d\n", fd[i]);
    }

    printf("fds opened\n");

    comp_list = create_completion_list(fd[0]);
    if (comp_list == NULL)
    {
        printf("error in create_completion_list\n");
        return -1;
    }
    printf("compeltion list created\n");

    args work1;
    work1.i = 1;
    work1.fd = fd[0];
    if (add_worker_thread(work1.fd, &work, &work1, comp_list->id) != 0)
    {
        printf("error in add_worker_thread\n");
        return -1;
    }
    //printf("work 1 added\n");
    workers_added++;

    args work2;
    work2.i = 2;
    work2.fd = fd[0];
    if (add_worker_thread(work2.fd, &work, &work2, comp_list->id) != 0)
    {
        printf("error in add_worker_thread 2\n");
        return -1;
    }
    //printf("work 2 added\n");
    workers_added++;

    if (!proc_test)
    {
        args work_multiple[NUM_WORKERS];
        for (int j = 2; j < NUM_WORKERS; j++)
        {
            work_multiple[j].i = j;
            work_multiple[j].fd = fd[0];
            if (add_worker_thread(work_multiple[j].fd, &work, &work_multiple[j], comp_list->id) != 0)
            {
                printf("error in add_worker_thread :%d\n", j);
            }
            //printf("work %d aggiunto\n", j);
            workers_added++;

            b++;
        }
    }

    for (int j = 0; j < NUM_THREAD; j++)
    {

        p_arg[j].fd = fd[j % NUM_UMS];

        p_arg[j].comp_list_id = comp_list->id;
        p_arg[j].padre = j;
        printf("genrated thread:%d, su comp_list:%lld, su fd:%d\n", p_arg[j].padre, p_arg[j].comp_list_id, p_arg[j].fd);

        if (pthread_create(&thread[j], NULL, &thread_func, &p_arg[j]) != 0)
        {
            printf("error in pthread create\n");

            break;
            return -1;
        }
        printf("genrated thread:%d, su comp_list:%lld, su fd:%d\n", p_arg[j].padre, p_arg[j].comp_list_id, p_arg[j].fd);
    }
    printf("thread created :D\n");

    if (!proc_test)
    {
        args work_multiple_2[NUM_WORKERS];
        for (int j = 0; j < NUM_WORKERS; j++)
        {
            work_multiple_2[j].i = j;
            work_multiple_2[j].fd = fd[0];
            if (add_worker_thread(work_multiple_2[j].fd, &work, &work_multiple_2[j], comp_list->id) != 0)
            {
                printf("error in add_worker_thread :%d\n", j);
            }
            //printf("work %d aggiunto\n", j);
            workers_added++;

            b++;
        }
    }
    printf("%d worker added\n", workers_added);


    destroy_comp_list(fd[0], comp_list->id);

    for (i = 0; i < NUM_UMS; i++)
    {
        printf("im about to release:%d\n", fd[i]);
        release_ums(fd[i]);
    }

    printf("total worker added:%d\n", b);
    printf("total worker executed:%d\n", a);
    return 0;
}
