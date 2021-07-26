#include "ums_user_library.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/**
 * @brief Struct used by a UMS Scheduler thread to mantain a pre-allocated completion_list_data* used in the dequeue function 
 * 
 */
typedef struct dequeue_fast_list
{

    struct completion_list_data *list_data;       /**< pointer to the completion_list_data that will be passed to the LKM*/
    int cur_size;                                 /**< cur size of list_data*/
    int origin_size;                              /**< origin size of list_data*/
    struct ums_worker_thread_data **list_pointer; /**< array that contains all the pointer used in list_data, this array is used only to free the element, since the kernel will overwrite ums_worker_thread_data->next in case of "NULL" */
} dequeue_fast_list;

int count_worker = 0, count_executed = 0;
__thread struct dequeue_fast_list fast_list;
pthread_mutex_t _library_mutex;

int fast_dequeue_list_init()
{
    fast_list.list_data = malloc(sizeof(completion_list_data));
    fast_list.list_data->head = NULL;
    fast_list.list_pointer = NULL;
    fast_list.cur_size = 0;
    fast_list.origin_size = 0;
    return 0;
}

int free_dequeue_output(completion_list_data *list_data)
{
    ums_worker_thread_data *worker = list_data->head;
    ums_worker_thread_data *aux;
    while (worker != NULL)
    {
        aux = worker;
        worker = worker->next;
        free(aux);
    }
    //free(list_data);
    list_data->head = NULL;
    return 0;
}

int fast_dequeue_list_free()
{
    //free_dequeue_output(fast_list.list_data);
    //free_dequeue_output(fast_list.list_pointer);
    for (int i = 0; i < fast_list.origin_size; i++)
    {
        free((*(fast_list.list_pointer + i)));
    }
    if (fast_list.list_pointer != NULL)
        free(fast_list.list_pointer);

    fast_list.list_data->head = NULL;
    fast_list.cur_size = 0;
    fast_list.origin_size = 0;
    return 0;
}

int fast_dequeue_list_alloc(struct completion_list_data *aux)
{

    struct ums_worker_thread_data *worker_aux;

    fast_list.list_data->id = aux->id;

    fast_list.list_pointer = (ums_worker_thread_data **)malloc(sizeof(ums_worker_thread_data *) * aux->requested_size);

    if (fast_list.list_pointer == NULL)
    {
        ERR_USER_PRINTF("cant alloc list_pointer\n");
        return -1;
    }

    for (int i = 0; i < aux->requested_size; i++)
    {

        if (fast_list.list_data->head == NULL)
        {

            fast_list.list_data->head = (ums_worker_thread_data *)malloc(sizeof(ums_worker_thread_data));

            if (fast_list.list_data->head == NULL)
            {
                ERR_USER_PPRINTF("ERROR: can't malloc\n");
                return -1;
            }

            worker_aux = fast_list.list_data->head;

            worker_aux->next = NULL;

            *(fast_list.list_pointer + i) = worker_aux;
            continue;
        }

        worker_aux->next = (ums_worker_thread_data *)malloc(sizeof(ums_worker_thread_data));

        if (worker_aux->next == NULL)
        {
            ERR_USER_PPRINTF("ERROR: can't malloc\n");
            return -1;
        }
        worker_aux = worker_aux->next;
        worker_aux->next = NULL;
        *(fast_list.list_pointer + i) = worker_aux;
    }
    fast_list.cur_size = aux->requested_size;
    fast_list.origin_size = aux->requested_size;

    fast_list.list_data->requested_size = aux->requested_size;

    return 0;
}

int ums_init()
{
    int fd;
    fd = open(OPEN_PATH, O_RDONLY);
    if (fd < 0)
    {
        ERR_USER_PPRINTF("ERROR: fd can't be open, err:%d\n", fd);
        return -1;
    }
    return fd;
}

int ums_close(int fd)
{
    int err;
    err = ioctl(fd, DESTROY_COMP_LIST, 0);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERORR: can't free list, err:%d\n", err);
        return err;
    };
    err = ioctl(fd, RELEASE_UMS, 0);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERORR: can't free UMS, err:%d\n", err);
        return err;
    };

    close(fd);
}

int enter_ums_scheduling_mode(int fd, int (*entry_point)(void *), void *entrypoint_args, ums_pid_t completion_list_id)
{
    //scheduler thread main loop
    int err;
    struct sched_thread_data new_sched_data;
    new_sched_data.completion_list = completion_list_id;
    new_sched_data.entry_point = entry_point;
    new_sched_data.args = entrypoint_args;

    err = ioctl(fd, ENTER_UMS_SCHEDULING_MODE, &new_sched_data);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR: can't become an UMS scheduler thread,err:%d\n", err);
        return err;
    }
    if (fast_dequeue_list_init() != 0)
    {
        ERR_USER_PRINTF("error:fast_dequeue_list_initi\n");
        return -1;
    }
    while (1)
    {
        //ERR_USER_PPRINTF("prima di ENTRYPOINT\n");
        err = entry_point(entrypoint_args);
        //printf("enter ums 1\n\n\n");
        //sleep(1);

        if (err < 0)
        {
            //printf("enter ums 2\n\n");
            ERR_USER_PPRINTF("ERROR, entry point function returned:%d\n", err);
            return err;
        }
    }
    return 0;
}

completion_list_data *dequeue_ums_completion_list_items(int fd)
{
    int err;
    struct completion_list_data *aux = malloc(sizeof(completion_list_data));

    // struct ums_worker_thread_data *worker_aux;

    if (aux == NULL)
    {
        ERR_USER_PPRINTF("Cant malloc dequeue\n");
        return NULL;
    }

    err = ioctl(fd, DEQUEUE_SIZE_REQUEST, aux);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR: can't retrive the size of the current list, err:%d\n", err);
        return NULL;
    }

    aux->head = NULL;

    if (fast_list.cur_size < aux->requested_size)
    {
        //printf("realloc\n");
        err = fast_dequeue_list_free();
        if (err < 0)
        {
            ERR_USER_PRINTF("error in fast_dequeue_list_free fot dequeue\n");
            return NULL;
        }
        
        err = fast_dequeue_list_alloc(aux);
        if (err < 0)
        {
            ERR_USER_PRINTF("error in fast_dequeue_list_alloc fot dequeue\n");
            return NULL;
        }
        
    }
    else
    {
        //ERR_USER_PRINTF("else\n");
        fast_list.cur_size = aux->requested_size;
        fast_list.list_data->requested_size = aux->requested_size;
    }
    //ERR_USER_PPRINTF("i have allocated enough memory for the kernel, im satisfied\n");
    err = ioctl(fd, DEQUEUE_UMS_COMPLETION_LIST_ITEMS, fast_list.list_data);

    if (err < 0 && errno == 404)
    {

        ERR_USER_PPRINTF("ERROR:i have to die ,err:%d\n", err);
        return NULL;
    }
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR: can't retrive the current list, err:%d\n", err);
        return NULL;
    }
    //ERR_USER_PPRINTF("i have allocated enough memory for the kernel, im full now\n");
    return fast_list.list_data;
}

int execute_ums_thread(int fd, ums_pid_t worker_id, completion_list_data *list_data)
{
    int err;
    struct ums_worker_thread_data aux;
    aux.pid = worker_id;
    err = ioctl(fd, EXECUTE_UMS_THREAD, &aux);
    if (err < 0 && errno == EBADF)
    {
        ERR_USER_EXEC_PPRINTF("ERROR: worker not found, try again pid:%llu\n", worker_id);
        return -errno;
    }
    if (err < 0 && errno == EACCES)
    {
        ERR_USER_EXEC_PPRINTF("ERROR: worker already running,try again pid:%llu\n", worker_id);
        return -errno;
    }
    if (err < 0 && errno == EFBIG)
    {
        ERR_USER_PPRINTF("ERROR: cant alloc:%llu\n", worker_id);
        return -errno;
    }
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR: execute_ums_thread pid:%llu\n", worker_id);
        return err;
    }
    //ERR_USER_PPRINTF("before actual work\n");
    //ERR_USER_PPRINTF("work: %d args: %d\n", aux.work, aux.args);
    //ERR_USER_PPRINTF("pid: %d completion_list: %d\n", aux.pid, aux.completion_list_id);
    //free_dequeue_output(list_data);
    aux.work(aux.args);
    err = exit_worker_thread(fd);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR, exit_worker_thread returned :%d\n", err);
        return err;
    }
    return 0;
}

int ums_thread_yield(int fd)
{
    int err, err1;
    struct sched_thread_data aux;
    err = ioctl(fd, UMS_THREAD_YIELD, &aux);

    if (err < 0 && errno == EBUSY)
    {

        DEBUG_USER_DO_PPRINTF("no worker found, continue execute\n");
        return 0;
    }
    else if (err < 0 && errno == EAGAIN)
    {
        //ERR_USER_PPRINTF("ums_yield worker founded\n");
        err1 = aux.entry_point(aux.args);
        //return 0;
        if (err1 < 0)
        {
            ERR_USER_PPRINTF("ERROR, entry point function returned from yield:%d\n", err1);
            //return err1;
        }

        err1 = ioctl(fd, EXIT_FROM_YIELD, 0);
        if (err1 < 0)
        {
            ERR_USER_PPRINTF("ERROR,exit from yield:%d\n", err1);
            return err1;
        }

        return 0;
    }
    else if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR, ums_thread_yield returned:%d\n", err);
        return err;
    }
    return 0;
}

int exit_worker_thread(int fd)
{
    //ERR_USER_PPRINTF("exit_thread\n");

    int err, err1;
    struct sched_thread_data aux;
    err = ioctl(fd, EXIT_WORKER_THREAD, &aux);
    pthread_mutex_lock(&_library_mutex);
    count_executed++;
    pthread_mutex_unlock(&_library_mutex);
    //ERR_USER_PPRINTF("exist_thread, err:%d\n", err);
    if (err < 0 && errno == EBUSY)
    {
        //ERR_USER_PPRINTF("ho nesting\n");
        DEBUG_USER_DO_PPRINTF("return  from yield\n");
        return 0; //vuol dir eche ho nesting, che devo fare?
    }
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR, exit_worker_thread returned:%d\n", err);
        return err;
    }
    // ERR_USER_PPRINTF("Sto per esegure la entry:%d\n", aux.entry_point);
    //ERR_USER_PPRINTF("args:%d\n", aux.args);
    err1 = aux.entry_point(aux.args);
    // ERR_USER_PPRINTF("eseguito la entry\n");
    if (err1 < 0)
    {
        ERR_USER_PPRINTF("ERROR, entry point function returned from exit:%d\n", err1);
        //return err1;
    }

    return 0;
}

completion_list_data *create_completion_list(int fd)
{
    int err;
    struct completion_list_data *aux = malloc(sizeof(completion_list_data));
    if (aux == NULL)
    {
        return NULL;
    }
    err = ioctl(fd, CREATE_COMP_LIST, aux);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERROR: can't create a new completion list, err:%d\n", err);
        return NULL;
    }
    return aux;
}

int add_worker_thread(int fd, int (*work)(void *), void *args, ums_pid_t completion_list_id)
{
    int err;
    ums_worker_thread_data aux;
    aux.completion_list_id = completion_list_id;
    aux.work = work;
    aux.args = args;
    err = ioctl(fd, ADD_WORKER, &aux);
    if (err < 0 && errno == EACCES)
    {
        ERR_USER_PPRINTF("the completion list is about to be destoy\n");
        return err;
    }
    if (err < 0)
    {
        ERR_USER_PPRINTF("can't add a new worker\n");
        return err;
    }
    pthread_mutex_lock(&_library_mutex);
    count_worker++;
    pthread_mutex_unlock(&_library_mutex);
    return 0;
}


int destroy_comp_list(int fd, ums_pid_t completion_list_id)
{
    //sleep(1);
    int err;
    completion_list_data aux;
    aux.id = completion_list_id;
    err = ioctl(fd, DESTROY_COMP_LIST, &aux);
    if (err < 0)
    {
        ERR_USER_PPRINTF("can't destroy the completion list,err:%d\n", err);
        return err;
    }
    //ERR_USER_PPRINTF("destroy the completion list executed,err:%d\n", err);
    return 0;
}

int release_ums(int fd)
{

    while (1)
    {
        usleep(500);
        pthread_mutex_lock(&_library_mutex);
        if (count_worker == count_executed)
        {
            pthread_mutex_unlock(&_library_mutex);
            break;
        }

       
         ERR_USER_EXEC_PPRINTF("ERORR: [w %d-%d ex], schedulers are currently running, ill sleep for 5sec\n", count_worker, count_executed);
        pthread_mutex_unlock(&_library_mutex);
    }
    int err = ioctl(fd, RELEASE_UMS, 0);
    if (err < 0)
    {
        ERR_USER_PPRINTF("ERORR: can't free UMS, err:%d\n", err);
        return err;
    };
    close(fd);
    return 0;
}
