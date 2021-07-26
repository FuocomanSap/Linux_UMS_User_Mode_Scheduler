//#include <linux/init.h>
#ifndef DEF_STRUCT_H_DEF
#define DEF_STRUCT_H_DEF


#include <linux/slab.h>
#include "user_struct.h"
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/spinlock.h>


/**
 * @file def_struct.h
 * @brief Contains the definitions of every custom type used in the Kernel module code
*/


#define EXIT 0
#define RUNNING 1
#define RUNNABLE 2
#define IOWAIT 3
#define SYSWAIT 4
#define YIELD 5
#define UNITIALIZED 6

#define PRIO 1
#define NOT_PRIO 0

/**
 * @brief structure containing the data needed by module to initialize the UMS device driver
 */
typedef struct ums_device_data_s {
    struct class*  class;
    struct device* device;
    int major;                     
} ums_device_data_t;


/**
 * @brief Strucute used by a UMS Scheduler to mantain Worker Thread info, used in ADD_WORKER
 * 
 * 
 */
typedef struct worker_thread
{   
    int state;                              /**<State of the worker */
    ums_pid_t pid;                          /**< id of the worker   */
    unsigned long completion_list_id;       /**<id of the completion list who own this this worker */
    spinlock_t execute_lock;                /**<used for concurrency while managing worker */
    struct list_head next;                  
    struct task_struct* task_struct;        /**<task struct related to this worker */
    int (*work) (void *);                   /**<function to be executed from this worker */
    void* args;                             /**< arguments for the work function */ 
    unsigned long numb_switch;              /**<args that will be passed to the work() function */
    time64_t runnning_time;
    time64_t starting_time;
}worker_thread;



/**
 * @brief Structure used by an UMS Scheduler to mantain Completion list infos, used in CREATE_COMP_LIST
 * 
 */
typedef struct completion_list
{   
    ums_pid_t id;                   /**<id of current completion list */
    unsigned char to_destroy;       /**<bool used by DESTROY_COMP_LIST to denie new worker insertion  */
    spinlock_t list_lock;           /**<used for concurrency while add new worker */
    struct semaphore sem_counter;   /**<used by a scheduler thread to be sure that some worker can be executed */
    struct list_head next;          
    struct list_head workers;       /**<reference to the worker threads list */

}completion_list;


/**
 * @brief Strucute used by a UMS Scheduler thread to save the order of the worker threads called
 * 
 */
typedef struct scheduler_thread_worker_lifo{
    struct worker_thread* this_worker;          
    struct list_head next;

}scheduler_thread_worker_lifo;



/**
 * @brief Structure used by an UMS Scheduler to mantain scheduler thread infos, used in ENTER_SCHEDULING_MODE
 * 
 */
typedef struct scheduler_thread
{
    pid_t pid;                                  /**< PID of the original pthread */
    int (*entry_point) (void *);                /**< scheduling function */ 
    void * args;                                /**< scheduling function */
    int yield_priority;                         /**< flag used for determining if the current scheduler thread has already done a down(&sem) */
    int nesting;                                /**< used with completion_list->sem_counter to take priority on the completion_list after a yield */
    struct worker_thread* currently_running;    /**< pointer to the currently running worker */
    //struct task_struct* task_struct;
    struct list_head  worker_lifo;              
    struct list_head  next;
    struct completion_list* completion_list; 
    spinlock_t sched_lock;
    int destroy;                                /**<flag used for determing when the thread has to exit after a ums_release(fd) */
    int started;                                /**<counter of the worker thread which it has started executing */ 
    int ended;                                  /**<counter of the worker thread that has completed */
    int numb_switch;                            /**<counter of the worker thread that switched */
    struct proc_dir_entry *ent;                 /**<pointer to proc_entry for "/proc/ums/scheduler_id/schedulers/pid" */
    struct proc_dir_entry *ent_write;           /**<pointer to proc_entry for "/proc/ums/scheduler_id/schedulers/pid/workers" */
    struct proc_dir_entry *father;              /**<pointer to proc_entry for "/proc/ums/scheduler_id/schdulers" */
    time64_t last_switch_time;                  /**<time elapsed to complete the last context switch */
    time64_t mean_switch_time;              /**<time elapsed to complete the last context switch */
    
}scheduler_thread;



/**
 * @brief Structure used by the kernel module to mantain differrent UMS Scheduler
 * 
 */
typedef struct UMS_sched_data
{ 
    //struct completion_list* completion_list;
    struct list_head scheduler_thread;        /**<used to mantain al the UMS Scheduler threads related to this UMS Scheduler */
    //int (*entry_point) (void *);
    struct list_head  next;
    unsigned long owner;                     /**< pid of the owner */
    unsigned long int id_counter;            /**< ID of the UMS Scheduler(we are using filep) */
    struct proc_dir_entry *ent;              /**< pointer to proc_entry for "/proc/ums/id_counter" */
    struct proc_dir_entry *ent_write;        /**< pointer to proc_entry for "/proc/ums/id_counter/schedulers" */
    struct proc_dir_entry *father;           /**< pointer to proc_entry for "/proc/ums" */
    

}UMS_sched_data;

#endif