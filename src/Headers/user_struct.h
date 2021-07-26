#ifndef USER_STRUCT_H_DEF
#define USER_STRUCT_H_DEF

typedef unsigned long long ums_pid_t;
/**
 * @file user_struct.h
 * @brief Contains the definitions of every custom type used in user space 
*/

//struct used from user sapce to call the ENTER_UMS_SCHEDULING_MODE
/**
 * @brief Struct used to share sched_thread_data info with the kernel
 * 
 * 
 */
typedef struct sched_thread_data{
    unsigned long completion_list; /**< ID of the completion_list*/
    int (*entry_point) (void *);  /**< pointer to the scheduling function */
    void *args; /**< pointer to the entry_point params(usually you need to pass at least the file descriptor)*/
}sched_thread_data;


/**
 * @brief Struct used to share completion_list_data info with the kernel
 * 
 */
typedef struct completion_list_data
{
   ums_pid_t id; /**< identifier for the completion list */
    struct ums_worker_thread_data* head; /**< pointer to an array of ums_worker_thread_data */
    int requested_size; /**< size requested from the kernel to be allocated in user space for the DEQUEUE*/

}completion_list_data;



//struct used from user sapce to call the ADD_WORKER
/**
 * @brief Struct used to share ums_worker_thread_data infos with the kernel
 *  
 */
typedef struct ums_worker_thread_data
{
   ums_pid_t pid; /**<identifier of the worker thread*/
   ums_pid_t completion_list_id; /**< unsigned long identifier for the completion list who own this worker*/
    int (*work) (void *); /**< pointer to a function that will be executed by the worker thread*/
    void* args; /**< argument that will be passed to work()*/
    struct ums_worker_thread_data* next; /**< pointer to the next worker (this variable is !null onli if retrived from a completion_list_data->head)*/
    //struct task_struct* task_struct;
}ums_worker_thread_data;


#endif