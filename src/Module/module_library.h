#ifndef MODULE_LIBRARY_H_DEF
#define MODULE_LIBRARY_H_DEF

#include <linux/sched.h>
#include <linux/uaccess.h>

#include "../Headers/def_struct.h"
#include "../Headers/Common.h"
#include <linux/signal.h>
#include "../Headers/user_struct.h"
#include <linux/sched/signal.h>
#include <linux/delay.h>
#include "../Headers/export_proc_info.h"
//#include <asm/siginfo.h>    //siginfo

//#define SIG_TEST SIGUSR1
//#define SIG_TEST1 SIGQUIT


/**
 * @file module_library.h
 * @brief Contains the helper functions called from the UMS device
*/


//--------------------------------//


/**
 * @brief Create a new scheduler_thread struct, by relating entry_point and _completion_list
 * 
 * @param sched_data pointer to the UMS Scheduler global variable that manage all the UMS Scheduler
 * @param filep      identifier of the file descriptoy   
 * @param entry_point scheduling function
 * @param args pointer to the args to pass to the entry_point function
 * @param comp_lists pointer to the UMS Scheduler global variable that manage all the completion lists
 * @param _completion_list identifier of the completion list to relate to this scheduler thread
 * @return int 0 on success err otherwise
 * for the possible err values, they are same of the relative ioctl
 */
int create_ums_thread(struct list_head* sched_data,unsigned long int filep ,int (*entry_point) (void *),void* args,struct list_head* comp_lists, ums_pid_t _completion_list );



/**
 * @brief retive the completion list managed by the current scheduler_thread
 * 
 * @param sched_data pointer to the UMS Scheduler global variable that manage all the UMS Scheduler
 * @param filep   identifier of the file descriptoy   
 * @return struct completion_list* on success NULL otherwise
 */
struct completion_list* retrive_current_list(struct list_head *sched_data,unsigned long int filep );


/**
 * @brief Retrive the current scheduler_thread structure
 * 
 * @param sched_data pointer to the UMS Scheduler global variable that manage all the UMS Scheduler
 * @param filep   identifier of the file descriptoy   
 * @return struct scheduler_thread* on success NULL otherwise
 */
struct scheduler_thread* retrive_current_scheduler_thread(struct list_head *sched_data,unsigned long int filep );


/**
 * @brief release the current UMS Scheduler and the relative compeltion_lists(if possibile) and scheduler_threads, if the schedulers are currently executing a worker this function returs -5;
 * 
 * @param sched_data pointer to the UMS Scheduler global variable that manage all the UMS Scheduler
 * @param filep   identifier of the file descriptoy  
 * @return int 0 on success err otherwise
 */
int release_scheduler(struct list_head* sched_data,unsigned long int filep);



/**
 * @brief Retrive the desired completion_list
 * 
 * @param comp_lists pointer to the UMS Scheduler global variable that manage all the completion lists
 * @param id id of the completion_list choosen
 * @return struct completion_list* on success, NULL otherwise
 */
struct completion_list* retrive_list(struct list_head* comp_lists,ums_pid_t id);

/**
 * @brief retrive the desired worker_thread
 * 
 * @param current_list pointer to the scheduler_thread->completion_list
 * @param pid id of the worker_thread
 * @return struct worker_thread* on success, NULL otherwise
 */
struct worker_thread* retrive_worker_thread(struct completion_list* current_list,ums_pid_t pid);
//---------------------//




/**
 * @brief Create a copy of the current completion list for the user 
 * 
 * @param to_write pointer to a completion_list_data struct, given by the user
 * @param from_read pointer to a completion_list struct, given by the kernel
 * @return int 0 on success, err otherwise
 */
int list_copy_to_user(struct completion_list_data* to_write,struct completion_list* from_read);



/**
 * @brief free the desired worker
 * 
 * @param worker_list the list_head who own this worker
 * @param worker_to_del pointer to the worker to del
 * @return int 0 on success, -1 otherwise
 */
int del_worker_thread(struct list_head worker_list, struct worker_thread *worker_to_del);


/**
 * @brief retrive the infos form the worker's struct in a char*
 * 
 * @param pid the selected worker id
 * @param comp_lists pointer to a Completion list list_head where the worker is stored
 * @return char* !=NULL on success, NULL otherwise
 */
char* _retrive_worker_thread_proc(time64_t pid,struct list_head *comp_lists);

/**
 * @brief retrive the infos form the schduler_thread's struct in a char*
 * 
 * @param pid the selected schduler_thread id
 * @param sched_data pointer to a UMS Scheduler list_head where the scheduler thread is stored
 * @return char* !=NULL on success, NULL otherwise
 */
char* _retrive_schduler_thread_proc(time64_t pid, struct list_head* sched_data);


/**
 * @brief Destroy the choosen comp list with id:completion_list_id
 * @param arg pointer to teh completion_list_data* that contains the info of the choosed completion list
 * @param comp_lists pointer to the list_head who own all the completion_list for this UMS Scheduler
 * @param completion_list_id the id of the completion list to destroy
 * @return int 0 on success, err otherwise, if err=ENOMEM no more space in the kernel
 */
int destoy_comp_list_set_destroy(unsigned long arg,struct list_head *comp_lists);


/**
 * @brief This function generate a new worker_thread form ums_worker_thread_data and then add it  to the desired completion_list
 * @param arg pointer to a user space ums_worker_thread_data * that contains the new woker infos
 * @param worker_to_add worker passed from user space
 * @param new_worker_thread struct allocated from the kernel to be filled
 * @param worker_counter monotonic counter of the worker
 * @param comp_lists pointer to the completion_list list_head who manage the choosen completion_list
 * @return int 0 on succees, err otherwise,if err=EACCES the completion list is marked to be destroyied.
 */
int add_worker(unsigned long arg,  ums_pid_t worker_counter, struct list_head *comp_lists);


/**
 * @brief This functions delete the worker and pass to the user new entry_point function
 * 
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @param arg pointer passed fro user space
 * @return int 0 on success, err otherwise, if err=-EBUSY :no other worker to execute
 */
int exit_worker_thread(struct list_head *sched_data, unsigned long int filep, unsigned long arg);


/**
 * @brief This function update the running time of thw worker and decrement the nesting variable
 * 
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @return int 0 on success, err otherwise
 */
int exit_from_yield(struct list_head *sched_data, unsigned long int filep );


/**
 * @brief Pause the execution of the current worker and write in (sched_thread_data*)arg entry_point and args
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @param arg pointer to a user space sched_thread_data* taht will be fill with entry_point and *args
 * @return errno
 * * errno:
         * - EBUSY, no other workers, this mean that you should just call the EXIT_FROM_YIELD
         * - EAGAIN, worker found, you have to call the entrypoint before call the EXIT_FROM_YIELD
          
 */
int ums_thread_yield(struct list_head *sched_data, unsigned long int filep, unsigned long arg);


/**
 * @brief Retrive from the user te id of the worker to execute, if found save the worker in the UMS SCheduler thread list of worker and pass to the user the work and args
 * 
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @param arg pointer to a user space sched_thread_data* passed with the id of the choosed woker, than  will be filled with entry_point and *args
 * @param worker_lifo_aux structure that will contain the choosed wroker to be added to the UMS SCheduler thread
 * @param cur_scheduler pointer to the current UMS Scheduler thread
 * @return int  0 on success, err otherwise.
         * err:
         * - -EFBIG, no more space in kernel to allow a new scheduler_thread_worker_lifo
         * - -EBADF, worker not found
         * - -EACCES, the worker is not RUNNABLE
         *  with one of this errno the default operation is to try to execute the next worker;
         * 
 */
int _execute_ums_thread(struct list_head *sched_data, unsigned long int filep, unsigned long arg,struct scheduler_thread *cur_scheduler,struct scheduler_thread_worker_lifo *worker_lifo_aux);

/**
 * @brief this functions is a wrapper for _execute_ums_thread, to manage the data statistics
 * 
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @param arg pointer to a user space sched_thread_data* passed with the id of the choosed woker, than  will be filled with entry_point and *args
 * @param worker_lifo_aux structure that will contain the choosed wroker to be added to the UMS SCheduler thread
* @return int  0 on success, err otherwise.
         * err:
         * - -EFBIG, no more space in kernel to allow a new scheduler_thread_worker_lifo
         * - -EBADF, worker not found
         * - -EACCES, the worker is not RUNNABLE
         *  with one of this errno the default operation is to try to execute the next worker;
         * 
 
 */
int execute_ums_thread(struct list_head *sched_data, unsigned long int filep, unsigned long arg, struct scheduler_thread_worker_lifo *worker_lifo_aux);



/**
 * @brief This function return the size to be allocated in user space in order to recive the list of ums_worker_thread_data(user space struct)
 * 
 * 
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @param arg pointer to a user space completion_list_data* used to set the requeste_size var
 * @return int 0 on success, err otherwise
 */
int dequeue_size_request(struct list_head *sched_data, unsigned long int filep,unsigned long arg);


/**
 * @brief This function converts a standard pthread in a UMS Scheduler thread
 * 
 * @param sched_data pointer to a UMS Scheduler list_head who manage this scheduler thread
 * @param filep id of the UMS Scheduler
 * @param arg pointer to a user space sched_thread_data* used to read the information needed to convert a pthread into a UMS Scheduler threads
 * @param comp_lists pointer to the completion_list list_head who manage the choosen completion_list
 * @return 0 on success, err otherwise
 */
int enter_ums_scheduling_mode(struct list_head *sched_data, unsigned long int filep,unsigned long arg,struct list_head *comp_lists);
 
#endif