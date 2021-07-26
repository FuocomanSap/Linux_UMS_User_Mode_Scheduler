#ifndef COMMON_H_DEF
#define COMMON_H_DEF

//IOCTL commands and struct

//#include <asm/ioctl.h>

/**
 * @file Common.h
 * @brief Contains constants and macros used both by the module and the user library 
*/

#define DEVICE_NAME "ums_scheduler"
#define MODULE_NAME_LOG "UMS_SCHEDULER: "
#define CLASS_NAME "ums_scheduler_class"
#define OPEN_PATH "/dev/ums_scheduler"



#define DEBUG_M_VAR 0

/**
 * @brief function used by the kernel to print debug msgs
 * 
 */
#define DEBUG_DO_PPRINTK(fmt, ...)            \
    if (DEBUG_M_VAR)                          \
    {                                         \
        printk(KERN_INFO fmt, ##__VA_ARGS__); \
    }

/**
 * @brief function used by the kernel to print debug msgs
 * 
 */
#define DEBUG_DO_PRINTK(fmt)   \
    if (DEBUG_M_VAR)           \
    {                          \
        printk(KERN_INFO fmt); \
    }

/**
 * @brief function used by the kernel to print error msgs
 * 
 */
#define ERR_PPRINTK(fmt, ...) printk(KERN_ERR fmt, ##__VA_ARGS__);
/**
 * @brief function used by the kernel to print error msgs
 * 
 */
#define ERR_PRINTK(fmt) printk(KERN_ERR fmt);

#define NOTIFY_EXEC_ERR_M_VAR 0
/**
 * @brief function used by the kernel to print error_exec msgs
 * 
 */
#define NOTIFY_EXEC_ERR_PPRINTK(fmt, ...) if(NOTIFY_EXEC_ERR_M_VAR)printk(KERN_ERR fmt, ##__VA_ARGS__);


///////////////////////////////////////////////////////////////////////
/* IOCTL definition */
/* WARNING */
/* THE IOCTLS will use as default errno: EINVAL when general error occur */

#define MAGIC_K_VALUE 'K'

/**
 * @brief ENTER_UMS_SCHEDULING_MODE
 * 
 * @param sched_thread_data* pointer to sched_thread_data
 * @return 0 on success, err otherwise and errno is setted
         * 
         * reads from args *sched_thread_data with: 
         * -entrypoint 
         * -args(for the entry point)
         * -the completion list id
         * 
         * Then call create_ums_thread.
         * 
         * returns 0 on success, err otherwise, the errno is setted
         * 
        **/
#define ENTER_UMS_SCHEDULING_MODE _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 0, sizeof(sched_thread_data)) //the last param is the size of the inupute args

/**
 * @brief DEQUEUE_UMS_COMPLETION_LIST_ITEMS
 * 
 * @param completion_list_data* pointer to completion_list_data
 * @return 0 on success, err otherwise and errno is setted
 * 
        
         * 
         * This function read from args *completion_list_data
         * 
         * if the completion list is empty(no worker to be executed) the UMS scheduler thread will sleep
         * if the completion list is not empty:
         * 1)the funciotn will check the max space allowed to be written form completion_list_Data
         * 2)then copy (only the allowed infos) worker_thread(kernel structre) to each ums_worker_thread_data(user structure) entry in completion_list_data
         * if no more space or no more worker the function exits
         * return 0 on succedd, err otherwise errno wil be set
         * errno:
         * - 404: the UMS called the UMS_RELEASE, this pthread should stop calling the device
         *        we suggest to return 0(see ums_user_library.c example)
         *
        
 * 
 */
#define DEQUEUE_UMS_COMPLETION_LIST_ITEMS _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 1, 8UL)

/**
 * @brief EXECUTE_UMS_THREAD
 * 
 * @param ums_worker_thread_data* pointer to ums_worker_thread_data
 * @return 0 on success, err otherwise and errno is setted
 * 
 
         * called from a scheduler thread, it executes the passed worker thread by switching the entire context
         * 
         * this function takes as input a *ums_worker_thread_data
         * if the worker is not RUNNABLE or not exists the function exits
         * otherwise set all the information in the current scheduler and then return ums_worker_thread_data filled 
         * with at least *work and *args
         * The worker will be set to RUNNING
         * 
         * errno:
         * - EFBIG, no more space in kernel to allow a new scheduler_thread_worker_lifo
         * - EBADF, worker not found
         * - EACCES, the worker is not RUNNABLE
         *  with one of this errno the default operation is to try to execute the next worker;
         * 
         
 * 
 * 
 */
#define EXECUTE_UMS_THREAD _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 2, 8UL)

/**
 * @brief UMS_THREAD_YIELD
 * 
 * @param sched_thread_data* pointer to sched_thread_data
 * @return err and errno is setted
 * 
 
         * UMS_THREAD_YIELD
         * 
         *called from a worker thread, it pauses the execution of the current thread and the UMS scheduler entry point is executed for determining the next thread to be scheduled;
         * 
         * Takes as input *sched_thread_data and fill it with *entry_point and *args
         * The worker will be set to YIELD
         * 
         * * errno:
         * - EBUSY, no other workers, this mean that you should just call the EXIT_FROM_YIELD
         * - EAGAIN, worker found, you have to call the entrypoint before call the EXIT_FROM_YIELD
         * 

 * 
 */
#define UMS_THREAD_YIELD _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 3, 8UL)

/**
 * @brief EXIT_WORKER_THREAD
 * 
 * @param sched_thread_data* pointer to sched_thread_data
 * @return 0 on success, err otherwise and errno is setted
         * 
         * This functions must be called at the end of a worker_thread *work function.
         * This function takes as input *sched_thread_data
         * 
         * Then the worker will be set to EXIT, and rmeoved from:
         * the completion_list and from the list of the UMS scheduler thread.
         *
         * 
         * 
         * at the end like the execute pass sched_thread_data to the user filled with entry_point and args
         * 
         * errno:
         * - EBUSY, no other worker to execute, but you come from a UMS_THREAD_YIELD, 
         *          you have simply to return 0 from the current function;
         * 
         * 
 * 
 */
#define EXIT_WORKER_THREAD _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 4, 8UL)

/**
 * @brief CREATE_COMP_LIST
 * @param completion_list_data* pointer to completion_list_data
 * @return 0 on success, err otherwise and errno is setted
 * 
        
         * 
         * this function takes as input *completion_list_data
         * 
         * generate a new completion_list, and store the id in the passed *completion_list_data
         * 
         * return 0 on success, err otherwise and errno is setted
         * 
         * errno:
         * -ENOMEM no more space in the kernel
         * 
         * 
         
 * 
 */
#define CREATE_COMP_LIST _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 5, 8UL)


/**
 * @brief ADD_WORKER
 * 
 * @param ums_worker_thread_data* pointer to ums_worker_thread_data
 * @return 0 on success, err otherwise and errno is setted.
         * 
         * This function takes as input *ums_worker_thread_data
         * 
         * add the ums_worker_thread_data to the desired completion list
         * 
         * return 0 on success, err otherwise and errno is setted.
         * 
         * in the case of readers/writers or publish/subscribe or master/slaves use 2 different completion_list for the two groups of workers
         * 
         * errno:
         * -EACCES, the completion list is marked to be destroyied.
         * 
         * 
      
 * 
 */
#define ADD_WORKER _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 6, 8UL)


/**
 * @brief DESTROY_COMP_LIST
 * @param completion_list_data* pointer to a completion_list_data
 * @return 0 on success, err otherwise and errno is setted.
 
         * DESTROY_COMP_LIST
         * takes as input a *completion_list_data
         * this function simply mark the completion_list to destroy
         * 
        
 * 
 */
#define DESTROY_COMP_LIST _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 7, 8UL)


/**
 * @brief RELEASE_UMS
 * 
 * @return 0 on success, err otherwise and errno is setted.
         * RELEASE_UMS
         * 
         * this funciton simply relelase the current UMS Scheduler
         * 
        
 * 
 */
#define RELEASE_UMS _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 8, 8UL)
/**
 * @brief DEQUEUE_SIZE_REQUEST
 * 
 * @param pointer to completion_list_data
 * @return 0 on success, err otherwise and errno is setted
 * 
 * 
         * This function return the size to be allocated in user space
         * in order to recive the list of ums_worker_thread_data(user space struct)
         * 
         * copy to args *completion_list_data with: 
         * -requested_size setted
         *  
         
 */
#define DEQUEUE_SIZE_REQUEST _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 9, 8UL)
/**
 * @brief EXIT_FROM_YIELD
 * 
 * @return 0 on success, err otherwise and errno is setted
 * 
         * This is a helper function, should be called after an UMS_THREAD_YIELD
         * this simply decrement a yield counter in the scheduler
         * not calling this function will have no consequences on the exectuion
         * but scheudler infos will be messed up, so that RELEASE will fails when call 
  
 * 
 */
#define EXIT_FROM_YIELD _IOC(_IOC_WRITE | _IOC_READ, MAGIC_K_VALUE, 10, 8UL)

#endif