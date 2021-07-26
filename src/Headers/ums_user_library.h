#ifndef UMS_USER_LIBRARY_H_DEF
#define UMS_USER_LIBRARY_H_DEF
#include "Common.h"
#include "user_struct.h"
#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
/**
 * 
 * @file ums_user_library.h
 * 
 * @brief This library contains the definitions of every function that can be used from user space to interact with the UMS Scheduler device
*/

#define DEBUG_USER_M_VAR 0
/**
 * @brief function used by the library to print debug msgs
 * 
 */
#define DEBUG_USER_DO_PPRINTF(fmt, ...) \
    if (DEBUG_USER_M_VAR)               \
    {                                   \
        printf(fmt, ##__VA_ARGS__);     \
    }
    /**
 * @brief function used by the library to print debug msgs
 * 
 */
#define DEBUG_USER_DO_PRINTF(fmt) \
    if (DEBUG_USER_M_VAR)         \
    {                             \
        printf(fmt);              \
    }
#define ERR_USER_M_VAR 1
/**
 * @brief function used by the library to print err msgs
 * 
 */
#define ERR_USER_PPRINTF(fmt, ...) \
    if (ERR_USER_M_VAR)            \
        printf(fmt, ##__VA_ARGS__);

/**
 * @brief function used by the library to print err msgs
 * 
 */
#define ERR_USER_PRINTF(fmt) \
    if (ERR_USER_M_VAR)      \
        printf(fmt);


#define ERR_USER_EXEC_M_VAR 0
/**
 * @brief function used by the library to print err_exec msgs
 * 
 */
#define ERR_USER_EXEC_PPRINTF(fmt, ...) \
    if (ERR_USER_EXEC_M_VAR)            \
        printf(fmt, ##__VA_ARGS__);

/**
 * @brief function used by the library to print err_exec msgs
 * 
 */
#define ERR_USER_EXEC_PRINTF(fmt) \
    if (ERR_USER_EXEC_M_VAR)      \
        printf(fmt);        

/**
 * @brief Initialize a new UMS
 * 
 * 
 * @return int returns the file descriptor of the device driver, -1 otherwise
 */
int ums_init();

/**
 * @brief Release all the structures related to the fd UMS
 * 
 * 
 * @return int return 0 on success, -1 otherwise
 */
int ums_close(int fd);

/**
 * @brief Converts a standard pthread in a UMS Scheduler thread, the function takes as input a file descriptor, a completion list ID of worker threads and a entry point function 
 * 
 * @param fd File descriptor related to the UMS schduler
 * @param entry_point Pointer to a Scheduling function
 * @param completion_list_id ID of a completion list that will be schedule on this UMS Scheduler thread
 * @param entrypoint_args pointer to entry_point funtion's parameters
 * @return int returns 0 on success, -1 otherwise
 */
int enter_ums_scheduling_mode(int fd, int (*entry_point)(void *), void *entrypoint_args, ums_pid_t completion_list_id);

/**
 * @brief called from the scheduler thread obtains a list of current available thread to be run, if no thread is available to be run the function should be blocking until a thread becomes available
 * 
 *
 * @param fd  File descriptor related to the UMS
 * @return struct completionm_list_data* pointer to a completion_list_data  
 */
completion_list_data *dequeue_ums_completion_list_items(int fd);

/**
 * @brief called from a scheduler thread, it executes the passed worker thread by switching the entire context, if -EACCES or -EBADF are returned try again with a different worker_id. 
 * 
 * @param fd File descriptor related to the UMS
 * @param worker_id id of the worker to be executed, the id is contenuto in the struct ums_worker_thread_data contenua in the struct completion_list_data->head
 * @param list_data pointer to the completion_list retrived from the dequeue, this list will be free
 * @return int returns job's return value on success, -EBADF or -EACCES if a worker is already running, err otherwise
 */
int execute_ums_thread(int fd, ums_pid_t worker_id,completion_list_data *list_data);

/**
 * @brief called from a worker thread, it pauses the execution of the current thread and the UMS scheduler entry point is executed for determining the next thread to be scheduled;
 * 
 * @param fd File descriptor related to the UMS
 * @return int return 0 on success, -1 otherwise
 */
int ums_thread_yield(int fd);

/**
 * @brief This function must be called at the end of every worker_thread
 * 
 * @param fd File descriptor related to the UMS
 * @return int return 0 on success, -1 otherwise
 */
int exit_worker_thread(int fd);

/**
 * @brief Create a completion list object, the ID is located in create_completion_list->id, 
 * in the case of readers/writers or publish/subscribe or master/slaves use 2 different completion_list for the two groups of workers
 * 
 * @param fd File descriptor related to the UMS
 * @return completion_list_data* on success, NULL otherwise
 */
completion_list_data *create_completion_list(int fd);

/**
 * @brief Create and add a worker thread to the desired completion list
 * 
 * @param fd File descriptor related to the UMS
 * @param work pointer to a function that will be executed by the worker thread
 * @param args argument that will be passed to work()
 * @param completion_list_id unsigned long identifier for the completion list who will own this worker
 * @return int 0 on success, -1 otherwise
 */
int add_worker_thread(int fd, int (*work)(void *), void *args, ums_pid_t completion_list_id);

/**
 * @brief destroy the selected completion list
 * 
 * @param fd File descriptor related to the UMS
 * @param completion_list_id ID of the completion list to destroy
 * @return int 0 on success, -1 otherwise
 */
int destroy_comp_list(int fd, ums_pid_t completion_list_id);

/**
 * @brief destroy the UMS
 * 
 * @param fd File descriptor related to the UMS
 * @return int 0 on success, -1 otherwise
 */
int release_ums(int fd);

#endif