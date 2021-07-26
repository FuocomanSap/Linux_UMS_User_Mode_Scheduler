#include "ums.h"
#include "../Headers/Common.h"
#include <linux/proc_fs.h>

MODULE_AUTHOR("Francesco Douglas Scotti di Vigoleno - Daniele de Turris");
MODULE_DESCRIPTION("A linux kernel module for User Mode Scheduling ");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

ums_device_data_t ums_device_data = {
    .class = NULL,
    .device = NULL,
    .major = -1};

struct file_operations ums_ops = {
    .open = ums_dev_open,
    .release = ums_dev_release,
    .unlocked_ioctl = ums_dev_ioctls,
    .owner = THIS_MODULE};

int max_counter = 0;
ums_pid_t worker_counter = 0;
ums_pid_t completion_list_counter = 0;
struct list_head sched_data; //list of all UMS schedulers
struct list_head comp_lists;
//spinlock_t UMS_lock;
struct mutex UMS_mutex;
//spinlock_t COMPLETION_lock;
struct mutex COMPLETION_mutex;

struct proc_dir_entry *ums_ent;

/*

struct file_operations group_ops = {
   .open = group_dev_open,
   .read = group_dev_read,
   .write = group_dev_write,
   .release = group_dev_release,
    .flush = group_dev_flush,
   .unlocked_ioctl = group_dev_ioctls,
   .owner = THIS_MODULE
};
*/

/*
long max_message_size = DEFAULT_MESSAGE_SIZE;
module_param(max_message_size, long, S_IWUSR |  S_IRUSR |  S_IWGRP  | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(max_message_size, "Maximum size of a single message");
*/

int __init load_ums_module(void)
{
    int err_class;
    INIT_LIST_HEAD(&sched_data);
    INIT_LIST_HEAD(&comp_lists);
    //spin_lock_init(&UMS_lock);
    mutex_init(&UMS_mutex);
    //spin_lock_init(&COMPLETION_lock);

    mutex_init(&COMPLETION_mutex);

    create_proc_ums(&ums_ent);
    if (ums_ent == NULL)
    {
        ERR_PRINTK("umd_ent null\n");
    }
    err_class = 0;
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "loading module\n");
    //major
    ums_device_data.major = register_chrdev(0, DEVICE_NAME, &ums_ops);
    if (ums_device_data.major < 0)
    {
        ERR_PRINTK(MODULE_NAME_LOG "error in generatin MAJOR\n");
        return ums_device_data.major;
    }
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "MAJOR %d generated:\n", ums_device_data.major);

    //register class
    ums_device_data.class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ums_device_data.class))
    { // Check for error and clean up if there is
        err_class = 1;
        goto creation_class_failed;
    }
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "class generated: %p\n", ums_device_data.class);

    // Register the device driver
    //mkdev trasforma major,minor in un dev_t
    ums_device_data.device = device_create(ums_device_data.class, NULL, MKDEV(ums_device_data.major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ums_device_data.device))
        goto creation_dev_failed;

    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "device generated: %p\n", ums_device_data.device);

    DEBUG_DO_PPRINTK(KERN_INFO MODULE_NAME_LOG "correctly created\n");
    printk(MODULE_NAME_LOG "Mounted\n");
    //ums_ent=kmalloc(sizeof(proc_dir_entry),GFP_KERNEL);

    return 0;

creation_dev_failed:
    printk(MODULE_NAME_LOG "error 1\n");
    class_destroy(ums_device_data.class);
creation_class_failed:
    printk(MODULE_NAME_LOG "error 2\n");
    unregister_chrdev(ums_device_data.major, DEVICE_NAME);
    if (err_class)
        ERR_PPRINTK("Failed to register device class %s\n", CLASS_NAME);
    return err_class ? PTR_ERR(ums_device_data.class) : PTR_ERR(ums_device_data.device); // Correct way to return an error on a pointer
}

void __exit unload_ums_module(void)
{
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "unaload module started\n");
    device_destroy(ums_device_data.class, MKDEV(ums_device_data.major, 0)); // remove the device
    class_destroy(ums_device_data.class);                                   // remove the device class
    unregister_chrdev(ums_device_data.major, DEVICE_NAME);
    rm_proc_ums(); // unregister the major number
    DEBUG_DO_PRINTK(KERN_INFO MODULE_NAME_LOG ": unloaded module!\n");
    printk(MODULE_NAME_LOG "Unloaded\n");
    //TODO free della lista di ums, inclusa ogni sua sotto lista
}

int ums_dev_open(struct inode *inodep, struct file *filep)
{
    UMS_sched_data *new_ums;
    //unsigned long flags;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "ums_dev_open\n");
    //TODO: usare variabile globale e lock per creare un counter e id degli UMS scheduler
    //DEBUG_DO_PRINTK(MODULE_NAME_LOG "open called\n");

    /*
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    list_for_each(current_item_list, &sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        if (current_UMS->id_counter == current->tgid)
        {
            ERR_PRINTK(MODULE_NAME_LOG "You are trying to open twice in the same process\n");
            
            return 0;
        }
    }
    */

    new_ums = (UMS_sched_data *)kmalloc(sizeof(UMS_sched_data), GFP_KERNEL);
    //sched_data = kmalloc(sizeof(sched_data), GFP_KERNEL);
    if (new_ums == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "error in allocating sched_data\n");
        return -EINVAL;
    }
    //new_ums->id_counter = current->tgid; //ex current->pid
    new_ums->id_counter = (long unsigned int)filep;
    new_ums->owner = current->pid;
    max_counter++;

    //spin_lock_irqsave(&UMS_lock, flags);
    mutex_lock(&UMS_mutex);
    INIT_LIST_HEAD(&(new_ums->scheduler_thread));
    create_proc_ums_scheduler((long unsigned int)filep, new_ums, ums_ent);
    if ((new_ums->ent) == NULL)
    {
        ERR_PRINTK("ent NULL\n");
        //spin_unlock_irqrestore(&UMS_lock, flags);
        mutex_unlock(&UMS_mutex);

        return -EINVAL;
    }

    list_add(&new_ums->next, &sched_data);

    //spin_unlock_irqrestore(&UMS_lock, flags);
    mutex_unlock(&UMS_mutex);

    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "UMS device opened, number: %d\n", max_counter);
    //DEBUG_DO_PPRINTK(MODULE_NAME_LOG "richietsa ricevuta da: inodep:%d, filep:%d", inodep, filep);
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit ums_dev_open\n");

    return 0;
}

int ums_dev_release(struct inode *inodep, struct file *filep)
{
    //TODO realese padre deve madnare sigkill a tutti i suoi figli

    //cancellare lo UMS
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG ": UMS device closed, remain: %d UMS\n", max_counter);
    return 0;
}

long ums_dev_ioctls(struct file *filep, unsigned int cmd, unsigned long arg)
{
    long ret;
    int sem_down_ret;
    int err;
    int max_loop;

    //struct task_struct *curr_task_struct; // = get_pid_task(find_get_pid(current->pid), PIDTYPE_PID);
    //struct timespec64 ts, ts2;
    unsigned long flags; //, flags2, flags4;
    //-----------------//
    struct scheduler_thread *cur_scheduler;
    //-----------------//
    //struct sched_thread_data aux;
    //sched_thread_data *ums;
    //-----------------//
    struct scheduler_thread_worker_lifo *worker_lifo_aux;
    //-----------------//
    //struct list_head *current_worker;

    //-----------------//
    //struct worker_thread *worker, *worker_to_execute, *new_worker_thread;
    //-----------------//
    struct completion_list_data comp_list_arg, cld;
    //-----------------//
    //struct ums_worker_thread_data retrived_worker_to_execute, worker_to_add;
    //-----------------//
    struct completion_list *cur_list, *new_comp_list;
    //-----------------//
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "richietsa ricevuta per filep:%lu, da [pid:%d tgid:%d]\n", (long unsigned int)filep, current->pid, current->tgid);
    ret = 1;

    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "device_ioctl: pid->%d, tgid->%d, path=%s, request=%u\n", current->pid, current->tgid, filep->f_path.dentry->d_iname, cmd);

    switch (cmd)
    {
        /**
         * 
         * ENTER_UMS_SCHEDULING_MODE
         * 
         * reads from args *sched_thread_data with: 
         * -entrypoint 
         * -args(for the entry point)
         * -the completion list id
         * 
         * Then call create_ums_thread.
         * 
         * returns 0 on success, err otherwise, the errno is setteed
         * 
        **/
    case ENTER_UMS_SCHEDULING_MODE:
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "ENTER_UMS_SCHEDULING_MODE\n");
        //spin_lock_irqsave(&UMS_lock, flags);
        mutex_lock(&UMS_mutex);

        err = enter_ums_scheduling_mode(&sched_data, (long unsigned int)filep, arg, &comp_lists);
        //spin_unlock_irqrestore(&UMS_lock, flags);
        mutex_unlock(&UMS_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit:ENTER_UMS_SCHEDULING_MODE\n");
        return err;
        break;

        /***
         * DEQUEUE_SIZE_REQUEST
         *  
         * This function return the size to be allocated in user space
         * in order to recive the list of ums_worker_thread_data(user space struct)
         * 
         * copy to args *completion_list_data with: 
         * -requested_size setted
         *  
         ***/
    case DEQUEUE_SIZE_REQUEST:;
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "enter DEQUEUE_SIZE_REQUEST\n");
        //spin_lock_irqsave(&UMS_lock, flags4);
        mutex_lock(&UMS_mutex);

        err = dequeue_size_request(&sched_data, (long unsigned int)filep, arg);
        //spin_unlock_irqrestore(&UMS_lock, flags4);
        mutex_unlock(&UMS_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit DEQUEUE_SIZE_REQUEST\n");
        return err;
        break;

        /***
         * DEQUEUE_UMS_COMPLETION_LIST_ITEMS
         * 
         * This function read from args *completion_list_data
         * 
         * if the completion list is empty(no worker to be executed) the UMS scheduler thread will sleep
         * if the completion list is not empty:
         * 1)the funciotn will check the max space allowed to be written form completion_list_Data
         * 2)then copy (only the allowed infos) worker_thread(kernel structre) to each ums_worker_thread_data(user structure) entry in completion_list_data
         * if no more space or no more worker the function exits
         * 
         * return 0 on success, err otherwise errno wil be set
         * errno:
         * - 404: the UMS called the UMS_RELEASE, this pthread should stop calling the device
         *        we suggest to return 0(see ums_user_library.c example)
         *
         * 
         * 
        ***/
    case DEQUEUE_UMS_COMPLETION_LIST_ITEMS:;
        //This function cant be moved out because use some magic tricks with sem and spinlocks
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "DEQUEUE_UMS_COMPLETION_LIST_ITEMS\n");
        if ((completion_list_data *)arg == NULL)
        {
            ERR_PRINTK(MODULE_NAME_LOG "ERROR: EDEQUEUE_UMS_COMPLETION_LSIT_ITEMS invalid arg\n");
            return -EINVAL;
        }
        copy_from_user(&comp_list_arg, (completion_list_data *)arg, sizeof(completion_list_data));

        //spin_lock_irqsave(&UMS_lock, flags4);
        mutex_lock(&UMS_mutex);

        cur_scheduler = retrive_current_scheduler_thread(&sched_data, (long unsigned int)filep);

        if (unlikely(cur_scheduler == NULL))
        {
            //spin_unlock_irqrestore(&UMS_lock, flags4);
            mutex_unlock(&UMS_mutex);

            ERR_PRINTK(MODULE_NAME_LOG "No scheduler thread related to this UMS Scheduler thread\n");
            return -EINVAL;
        }

        cur_list = cur_scheduler->completion_list;
        if (unlikely(cur_list == NULL))
        {
            //spin_unlock_irqrestore(&UMS_lock, flags4);
            mutex_unlock(&UMS_mutex);

            ERR_PRINTK(MODULE_NAME_LOG "No completion list related to this UMS Scheduler thread\n");
            return -EINVAL;
        }

        if (likely(cur_scheduler->yield_priority == NOT_PRIO))
        {
            //spin_unlock_irqrestore(&UMS_lock, flags4);
            mutex_unlock(&UMS_mutex);

            DEBUG_DO_PRINTK(MODULE_NAME_LOG "SEM to be call\n");
            while (1)
            {
                sem_down_ret = down_interruptible(&(cur_list->sem_counter));
                //spin_lock_irqsave(&UMS_lock, flags4);
                mutex_lock(&UMS_mutex);
                cur_scheduler = retrive_current_scheduler_thread(&sched_data, (long unsigned int)filep);

                if (sem_down_ret == -EINTR || cur_scheduler == NULL)
                {
                    if (cur_scheduler == NULL)
                    {
                        DEBUG_DO_PPRINTK("cur_sched=NULL, correct!\n");
                        //spin_unlock_irqrestore(&UMS_lock, flags4);
                        mutex_unlock(&UMS_mutex);

                        return -404;
                    }

                    if (cur_scheduler->yield_priority == NOT_PRIO && cur_scheduler->destroy == 1 && cur_scheduler->nesting == 0)
                    {
                        DEBUG_DO_PPRINTK("cur_scheduler is not null, but i have to die\n");
                        //spin_unlock_irqrestore(&UMS_lock, flags4);
                        mutex_unlock(&UMS_mutex);

                        return -404;
                    }
                    //spin_unlock_irqrestore(&UMS_lock, flags4);
                    mutex_unlock(&UMS_mutex);

                    ERR_PRINTK("error down_interruptible recive a signal but not from father\n");
                }
                else
                {
                    //spin_unlock_irqrestore(&UMS_lock, flags4);
                    DEBUG_DO_PPRINTK("down_interruptible: success\n");
                    break;
                }
            }
            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "sem ret: %d\n", sem_down_ret);
        }

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "before list_copy_to_user\n");
        spin_lock_irqsave(&(cur_list->list_lock), flags);
        if (list_copy_to_user(&comp_list_arg, cur_list) < 0)
        {
            spin_unlock_irqrestore(&cur_list->list_lock, flags);
            //spin_unlock_irqrestore(&UMS_lock, flags4);
            mutex_unlock(&UMS_mutex);

            return -EINVAL;
        };
        spin_unlock_irqrestore(&cur_list->list_lock, flags);
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "after list_copy_to_user\n");

        copy_to_user((completion_list_data *)arg, &comp_list_arg, sizeof(struct completion_list_data));
        //spin_unlock_irqrestore(&UMS_lock, flags4);
        mutex_unlock(&UMS_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit DEQUEUE_UMS_COMPLETION_LIST_ITEMS\n");
        break;

        /***
         * EXECUTE_UMS_THREAD
         * 
         * 
         * called from a scheduler thread, it executes the passed worker thread by switching the entire context
         * 
         * this function takes as input a *ums_worker_thread_data
         * if the worker is not RUNNABLE or not exists the function exits
         * otherwise set all the information in the current scheduler and then return ums_worker_thread_data filled 
         * with at least *work and *args
         * 
         * The worker will be set to RUNNING
         * 
         * 
         * return 0 on success, err otherwise errno is setted
         * errno:
         * - EFBIG, no more space in kernel to allow a new scheduler_thread_worker_lifo
         * - EBADF, worker not found
         * - EACCES, the worker is not RUNNABLE
         *  with one of this errno the default operation is to try to execute the next worker;
         * 
         ***/
    case EXECUTE_UMS_THREAD:
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "EXECUTE_UMS_THREAD\n");

        //spin_lock_irqsave(&UMS_lock, flags4);
        mutex_lock(&UMS_mutex);

        err = execute_ums_thread(&sched_data, (long unsigned int)filep, arg, worker_lifo_aux);
        //spin_unlock_irqrestore(&UMS_lock, flags4);
        mutex_unlock(&UMS_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit EXECUTE_UMS_THREAD\n");
        return err;
        break;

        /**
         * UMS_THREAD_YIELD
         * 
         *called from a worker thread, it pauses the execution of the current thread and the UMS scheduler entry point is executed for determining the next thread to be scheduled;
         * 
         * Takes as input *sched_thread_data fill it with *entry_point and *args
         * The worker will be set to YIELD
         * 
         * errno:
         * - EBUSY, no other workers, this mean that you should just call the EXIT_FROM_YIELD
         * - EAGAIN, worker found, you have to call the entrypoint before call the EXIT_FROM_YIELD
         */
    case UMS_THREAD_YIELD:
        //called from a worker thread, it pauses the execution of the current thread and the UMS scheduler entry point is executed for determining the next thread to be scheduled;
        ;
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "UMS_THREAD_YIELD\n");
        //spin_lock_irqsave(&UMS_lock, flags4);
        mutex_lock(&UMS_mutex);

        err = ums_thread_yield(&sched_data, (long unsigned int)filep, arg);
        //spin_unlock_irqrestore(&UMS_lock, flags4);
        mutex_unlock(&UMS_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit UMS_THREAD_YIELD\n");
        return err;
        break;

        /***
         * EXIT_FROM_YIELD
         * 
         * This is a helper function, should be called after an UMS_THREAD_YIELD
         * this simply decrement a yield counter in the scheduler
         * not calling this function will have no consequences on the exectuion
         * but scheudler infos will be messed up, so that RELEASE will fails when called 
         * 
         * 
         ***/
    case EXIT_FROM_YIELD:
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "EXIT_UMS_THREAD_YIELD\n");

        //spin_lock_irqsave(&UMS_lock, flags4);
        mutex_lock(&UMS_mutex);

        err = exit_from_yield(&sched_data, (long unsigned int)filep);
        //spin_unlock_irqrestore(&UMS_lock, flags4);
        mutex_unlock(&UMS_mutex);
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit:EXIT_UMS_THREAD_YIELD\n");
        return err;
        break;

        /***
         * EXIT_WORKER_THREAD
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
         * **/
    case EXIT_WORKER_THREAD:
        // Oppure più semplicemente potete imporre che al termine venga chiamata una funzione ExitWorkerThread() che ritorna allo scheduler.
        ;
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "EXIT_WORKER_THREAD\n");

        //spin_lock_irqsave(&UMS_lock, flags4);
        mutex_lock(&UMS_mutex);

        err = exit_worker_thread(&sched_data, (long unsigned int)filep, arg);
        //spin_unlock_irqrestore(&UMS_lock, flags4);
        mutex_unlock(&UMS_mutex);

        return err;
        break;

        /***
         * CREATE_COMP_LIST
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
         ***/
    case CREATE_COMP_LIST:;
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "CREATE_COMP_LIST\n");

        new_comp_list = kmalloc(sizeof(completion_list), GFP_KERNEL);
        if (new_comp_list == NULL)
        {
            ERR_PRINTK(MODULE_NAME_LOG "ERROR NOT ENOUGH MEMORY FOR A KMALLOC, create_comp_list\n");
            return -ENOMEM;
        }

        new_comp_list->to_destroy = 0;
        spin_lock_init(&new_comp_list->list_lock);
        INIT_LIST_HEAD(&(new_comp_list->workers));
        sema_init(&(new_comp_list->sem_counter), 0);

        //spin_lock_irqsave(&COMPLETION_lock, flags);
        mutex_lock(&COMPLETION_mutex);

        new_comp_list->id = completion_list_counter++;
        list_add(&(new_comp_list->next), &comp_lists);
        cld.id = new_comp_list->id;
        copy_to_user((completion_list_data *)arg, &cld, sizeof(unsigned long *));
        DEBUG_DO_PPRINTK(MODULE_NAME_LOG "Generated new comp list:%llu\n", new_comp_list->id);

        //spin_unlock_irqrestore(&COMPLETION_lock, flags);
        mutex_unlock(&COMPLETION_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit CREATE_COMP_LIST\n");
        break;

        /***
         * ADD_WORKER
         * 
         * This function takes as input *ums_worker_thread_data
         * 
         * add the ums_worker_thread_data to the desired completion list
         * 
         * return 0 on success, err otherwise and errno is setted.
         * 
         * errno:
         * -EACCES, the completion list is marked to be destroyied.
         * 
         * 
         ***/
    case ADD_WORKER:
        //riceve un puntatore a ums_worker_thread_data
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "ADD_WORKER\n");
        //spin_lock_irqsave(&COMPLETION_lock, flags);
        mutex_lock(&COMPLETION_mutex);

        err = add_worker(arg, worker_counter++, &comp_lists);
        //spin_unlock_irqrestore(&COMPLETION_lock, flags);
        mutex_unlock(&COMPLETION_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit ADD_WORKER\n");
        return err;
        break;

        /***
         * DESTROY_COMP_LIST
         * takes as input a *completion_list_data
         * 
         * this function simply mark the completion_list to destroy
         * 
         * **/
    case DESTROY_COMP_LIST:

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "DESTROY_COMP_LIST\n");
        //spin_lock_irqsave(&COMPLETION_lock, flags4);
        mutex_lock(&COMPLETION_mutex);

        err = destoy_comp_list_set_destroy(arg, &comp_lists);
        //spin_unlock_irqrestore(&COMPLETION_lock, flags4);
        mutex_unlock(&COMPLETION_mutex);

        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit: DESTROY_COMP_LIST\n");
        return err;
        break;

        /***
         * RELEASE_UMS
         * 
         * this funciton simply relelase the current UMS Scheduler
         * 
         * 
         * **/
    case RELEASE_UMS:;
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "RELEASE_UMS\n");

        max_loop = 0; //max loop is 10;
        ret = -5;     //-5 is an helper value, see release_scheduler() for the spec
        do
        {
            msleep(500);
            //spin_lock_irqsave(&UMS_lock, flags);
            mutex_lock(&UMS_mutex);

            ret = release_scheduler(&sched_data, (long unsigned int)filep);
            //spin_unlock_irqrestore(&UMS_lock, flags);
            mutex_unlock(&UMS_mutex);

            max_loop++;
        } while (ret == -5 && max_loop < 10);

        if (ret < 0)
        {
            ERR_PRINTK(MODULE_NAME_LOG "ERROR UMS_DEV_RELEASE cant release\n");
            return -EINVAL;
        }

        max_counter--;
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit RELEASE_UMS\n");
        break;
    default:
        ret = -ENOTSUPP;
    }

    return 0;
}

char *retrive_worker_thread_proc(time64_t pid)
{
    //long unsigned flag1, flag2;
    char *buf_to_write;
    //spin_lock_irqsave(&UMS_lock, flag2);
    mutex_lock(&UMS_mutex);

    //spin_lock_irqsave(&COMPLETION_lock, flag1);
    mutex_lock(&COMPLETION_mutex);

    buf_to_write = _retrive_worker_thread_proc(pid, &comp_lists);
    //spin_unlock_irqrestore(&COMPLETION_lock, flag1);
    mutex_unlock(&COMPLETION_mutex);

    //spin_unlock_irqrestore(&UMS_lock, flag2);
    mutex_unlock(&UMS_mutex);

    return buf_to_write;

    /*
    struct timespec64 ts2;
    time64_t aux_time_var = 0;
    char *buf_to_write = kmalloc(sizeof(char) * BUFSIZE, GFP_KERNEL);
    if (buf_to_write == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "Error in kmalloc retrive_worker_thread_proc\n");
        return NULL;
    }
    //TODO chewck kmalloc

    long unsigned flag1, flag2;
    struct worker_thread *aux = NULL;
    spin_lock_irqsave(&UMS_lock, flag2);
    spin_lock_irqsave(&COMPLETION_lock, flag1);

    struct list_head *current_list_item, *current_worker_list;
    struct completion_list *current_list;
    struct worker_thread *current_worker;

    list_for_each(current_list_item, &comp_lists)
    {
        current_list = list_entry(current_list_item, completion_list, next);

        list_for_each(current_worker_list, &current_list->workers)
        {
            current_worker = list_entry(current_worker_list, worker_thread, next);

            if (current_worker->pid == pid)
            {

                if (current_worker->runnning_time == 0 && current_worker->starting_time != 0)
                {
                    ERR_PRINTK("1\n");
                    ktime_get_real_ts64(&ts2);
                    ERR_PPRINTK("[%ld-%ld]\n", ts2.tv_sec, current_worker->starting_time);
                    aux_time_var = ts2.tv_sec - current_worker->starting_time;
                }
                else if (current_worker->runnning_time != 0 && current_worker->starting_time == 0)
                {
                    //siamo in un caso di yield
                    ERR_PRINTK("2\n");
                    aux_time_var = current_worker->runnning_time;
                }
                else if (current_worker->runnning_time == 0 && current_worker->starting_time == 0)
                {
                    ERR_PRINTK("3\n");
                    aux_time_var = current_worker->runnning_time;
                }
                else
                {
                    ERR_PRINTK("4\n");
                    aux_time_var = current_worker->runnning_time + ts2.tv_sec - current_worker->starting_time;
                }
                sprintf(buf_to_write, "#switches:%lu\nrunning_time:%ld\nstate:%d\n", current_worker->numb_switch, aux_time_var, current_worker->state);
                spin_unlock_irqrestore(&COMPLETION_lock, flag1);
                spin_unlock_irqrestore(&UMS_lock, flag2);
                return buf_to_write;
            }
        }
    }

    ERR_PPRINTK(MODULE_NAME_LOG "cant retrive the worker for the proc\n");
    spin_unlock_irqrestore(&COMPLETION_lock, flag1);
    spin_unlock_irqrestore(&UMS_lock, flag2);
    return NULL;
    */
}

char *retrive_schduler_thread_proc(time64_t pid)
{
    char *buf_to_write;
    //long unsigned flag1, flag2;
    //spin_lock_irqsave(&UMS_lock, flag2);
    mutex_lock(&UMS_mutex);

    //spin_lock_irqsave(&COMPLETION_lock, flag1);
    mutex_lock(&COMPLETION_mutex);

    buf_to_write = _retrive_schduler_thread_proc(pid, &sched_data);
    //spin_unlock_irqrestore(&COMPLETION_lock, flag1);
    mutex_unlock(&COMPLETION_mutex);

    //spin_unlock_irqrestore(&UMS_lock, flag2);
    mutex_unlock(&UMS_mutex);

    return buf_to_write;

    /*
    char *buf_to_write = kmalloc(sizeof(char) * BUFSIZE, GFP_KERNEL);
    unsigned long pid_cur_run = 0;
    if (buf_to_write == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "Error in kmalloc retrive_schduler_thread_proc\n");
        return NULL;
    }
    long unsigned flag1, flag2;
    struct scheduler_thread aux;

    spin_lock_irqsave(&UMS_lock, flag2);
    spin_lock_irqsave(&COMPLETION_lock, flag1);

    struct list_head *current_ums_list;
    struct UMS_sched_data *current_ums;

    struct list_head *current_thread_list;
    struct scheduler_thread *current_thread;

    list_for_each(current_ums_list, &sched_data)
    {
        current_ums = list_entry(current_ums_list, UMS_sched_data, next);

        list_for_each(current_thread_list, &current_ums->scheduler_thread)
        {
            current_thread = list_entry(current_thread_list, scheduler_thread, next);
            if (current_thread->pid == pid)
            {

                if (current_thread->currently_running != NULL)
                    pid_cur_run = current_thread->currently_running->pid;
                sprintf(buf_to_write, "#switches:%d\ncompletion_list_id:%lu\ntime_for_Switch:%lu\nrunning:%lu\n", current_thread->started, current_thread->completion_list->id, current_thread->last_switch_time, pid_cur_run);
                spin_unlock_irqrestore(&COMPLETION_lock, flag1);
                spin_unlock_irqrestore(&UMS_lock, flag2);
                return buf_to_write;
            }
        }
    }

    spin_unlock_irqrestore(&COMPLETION_lock, flag1);
    spin_unlock_irqrestore(&UMS_lock, flag2);
    return NULL;
    */
}

module_init(load_ums_module);
module_exit(unload_ums_module);
