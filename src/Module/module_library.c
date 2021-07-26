

#include "module_library.h"

/*
int create_ums_thread(struct list_head *sched_data, int (*entry_point)(void *), struct completion_list* completion_list)
{
    printk("ENTER create_ums_thread");
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    //we are looking for the UMS Scheduler related to the pid
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);
        //printk(MODULE_NAME_LOG "current_item->id=%d\n", current_UMS->id_counter);
        if (current_UMS->id_counter == current->tgid)
        {
            //printk("prima break");
            //TODO check se gia' esisto come ums_thread
            scheduler_thread *new_sched_thread = kmalloc(sizeof(scheduler_thread), GFP_KERNEL);
            //TODO mettere tid al posto del tid
            new_sched_thread->pid = current->pid; //we are adding the pid to the UMS Scheduler pid-entry
            new_sched_thread->entry_point = entry_point;
            new_sched_thread->completion_list = completion_list;
            struct task_struct* curr_task_struct = get_pid_task(find_get_pid(current->pid),PIDTYPE_PID);
            new_sched_thread->currently_running=NULL;
            new_sched_thread->task_struct=curr_task_struct;
            list_add(&new_sched_thread->next, &(current_UMS->scheduler_thread));
            printk(MODULE_NAME_LOG "associated UMS_Thread to the correct UMSScheduler,[UMSScheduler %d - %d UMS Scheduler Thread]\n", current_UMS->id_counter, new_sched_thread->pid);
            return 0;
        }
        printk("sto cercando");
    }
    return -1;
}

struct completion_list *retrive_current_list(struct list_head *sched_data)
{
    //TODO qui bisogna fare il check se sono peening o no
    printk("ENTER retrive_current_list");

    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        if (current_UMS->id_counter == current->tgid)
        {
            struct list_head *current_thread_list;
            scheduler_thread *current_thread;

            list_for_each(current_thread_list, &current_UMS->scheduler_thread)
            {
                current_thread = list_entry(current_thread_list, scheduler_thread, next);
                if(current_thread->pid==current->pid){
                    printk(MODULE_NAME_LOG "i have founded: [UMSScheduler %d - %d UMS Scheduler Thread]\n", current_UMS->id_counter, current->pid);
            
                    return current_thread->completion_list;
                }
            }
        }
    }
    return -1;
}


struct scheduler_thread *retrive_current_scheduler_thread(struct list_head *sched_data){
    //TODO verificare che questa cosa funzioni
    printk("ENTER retrive_current_list");

    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        if (current_UMS->id_counter == current->tgid)
        {
            struct list_head *current_thread_list;
            scheduler_thread *current_thread;

            list_for_each(current_thread_list, &current_UMS->scheduler_thread)
            {
                current_thread = list_entry(current_thread_list, scheduler_thread, next);
                if(current_thread->pid==current->pid){
                    printk(MODULE_NAME_LOG "i have founded: [UMSScheduler %d - %d UMS Scheduler Thread]\n", current_UMS->id_counter, current->pid);
            
                    return current_thread;
                }
            }
        }
    }
    return -1;
}


*/
int context_switch(struct task_struct *from, struct task_struct *to)
{

    //if to.tgid != from.tgid ERROR, stai esequendo un worken che non fa parte di mem pero' non va bene perche' ci deve essere condivisione fra UMS scheduler.
    return 0;
}

int create_ums_thread(struct list_head *sched_data, unsigned long int filep, int (*entry_point)(void *), void *args, struct list_head *comp_lists, ums_pid_t _completion_list)
{
    //printk("ENTER create_ums_thread");
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    scheduler_thread *new_sched_thread;
    struct task_struct *curr_task_struct;
    struct completion_list *aux;
    //we are looking for the UMS Scheduler related to the pid
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);
        //printk(MODULE_NAME_LOG "current_item->id=%d\n", current_UMS->id_counter);
        if (current_UMS->id_counter == filep)
        {
            //printk("prima break");
            //TODO check se gia' esisto come ums_thread
            new_sched_thread = kmalloc(sizeof(scheduler_thread), GFP_KERNEL);
            if (new_sched_thread == NULL)
            {
                ERR_PRINTK(MODULE_NAME_LOG "ERROR: kmalloc in create_ums_thread\n");
                return -EFBIG;
            }
            //TODO mettere tid al posto del tid

            new_sched_thread->pid = current->pid; //we are adding the pid to the UMS Scheduler pid-entry
            new_sched_thread->entry_point = entry_point;
            new_sched_thread->args = args;
            new_sched_thread->yield_priority = NOT_PRIO;
            new_sched_thread->nesting = 0;
            new_sched_thread->started = 0;
            new_sched_thread->numb_switch = 0;
            new_sched_thread->ended = 0;
            new_sched_thread->mean_switch_time = 0;
            INIT_LIST_HEAD(&new_sched_thread->worker_lifo);
            aux = retrive_list(comp_lists, _completion_list);

            new_sched_thread->completion_list = aux;
            if (aux < 0)
            {
                ERR_PPRINTK(MODULE_NAME_LOG "ERROR IN completion_list not founded:%llu\n", _completion_list);
                return -1;
            }
            spin_lock_init(&new_sched_thread->sched_lock);
            new_sched_thread->destroy = 0;
            //new_sched_thread->completion_list->a++;
            curr_task_struct = get_pid_task(find_get_pid(current->pid), PIDTYPE_PID);
            new_sched_thread->currently_running = NULL;
            //new_sched_thread->task_struct = curr_task_struct;
            list_add(&new_sched_thread->next, &(current_UMS->scheduler_thread));
            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "associated UMS_Thread to the correct UMSScheduler,[UMSScheduler %ld - %d UMS Scheduler Thread]\n", current_UMS->id_counter, new_sched_thread->pid);

            create_proc_ums_scheduler_thread(filep, new_sched_thread, current_UMS->ent_write);
            //write_proc_ums_scheduler_thread(filep,new_sched_thread);

            return 0;
        }
    }
    return -1;
}

struct completion_list *retrive_current_list(struct list_head *sched_data, unsigned long int filep)
{
    //TODO qui bisogna fare il check se sono peening o no
    //printk("ENTER retrive_current_list");

    /*
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        if (current_UMS->id_counter == filep)
        {
            struct list_head *current_thread_list;
            scheduler_thread *current_thread;

            list_for_each(current_thread_list, &current_UMS->scheduler_thread)
            {
                current_thread = list_entry(current_thread_list, scheduler_thread, next);
                if (current_thread->pid == current->pid)
                {
                    //DEBUG_DO_PRINTK(MODULE_NAME_LOG "crasha qui1\n");
                    if (current_thread->completion_list == NULL)
                    {
                        ERR_PRINTK(MODULE_NAME_LOG "il thread non ha nessun alist \n");
                        return NULL;
                    }
                    //printk(MODULE_NAME_LOG "i have founded: [UMSScheduler %d - %d UMS Scheduler Thread]\n", current_UMS->id_counter, current->pid);
                    printk(MODULE_NAME_LOG "i have founded: the list: %u\n", current_thread->completion_list->id);

                    return current_thread->completion_list;
                }
            }
        }
    }
    printk(MODULE_NAME_LOG "CURENT LIST NOT FOUNDED\n");
    return NULL;
    */
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    struct list_head *current_thread_list;
    scheduler_thread *current_thread;
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        list_for_each(current_thread_list, &current_UMS->scheduler_thread)
        {
            current_thread = list_entry(current_thread_list, scheduler_thread, next);
            if (current_thread->pid == current->pid)
            {

                if (current_thread->completion_list == NULL)
                {
                    ERR_PRINTK(MODULE_NAME_LOG "il thread non ha nessun alist \n");
                    return NULL;
                }
                DEBUG_DO_PPRINTK(MODULE_NAME_LOG "i have founded: the list: %llu\n", current_thread->completion_list->id);

                return current_thread->completion_list;
            }
        }
    }
    ERR_PRINTK(MODULE_NAME_LOG "CURENT LIST NOT FOUNDED\n");
    return NULL;
}

struct scheduler_thread *retrive_current_scheduler_thread(struct list_head *sched_data, unsigned long int filep)
{
    //TODO verificare che questa cosa funzioni
    //printk("ENTER retrive_current_list");
    /*
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        if (current_UMS->id_counter == filep)
        {
            struct list_head *current_thread_list;
            scheduler_thread *current_thread;

            list_for_each(current_thread_list, &current_UMS->scheduler_thread)
            {
                current_thread = list_entry(current_thread_list, scheduler_thread, next);
                if (current_thread->pid == current->pid)
                {
                    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "ho trovato il current scheduler:%d, con entry-args [%d-%d]", current_thread->pid, current_thread->entry_point, current_thread->args);
                    return current_thread;
                }
            }
        }
    }
    ERR_PRINTK(MODULE_NAME_LOG "cant retrive current shceduler thread\n");

    return NULL;
    */

    //////////////////////////////testing
    struct list_head *current_thread_list;
    scheduler_thread *current_thread;
    struct list_head *current_item_list;
    UMS_sched_data *current_UMS;
    list_for_each(current_item_list, sched_data)
    {
        current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        list_for_each(current_thread_list, &current_UMS->scheduler_thread)
        {
            current_thread = list_entry(current_thread_list, scheduler_thread, next);
            if (current_thread->pid == current->pid)
            {
                DEBUG_DO_PPRINTK(MODULE_NAME_LOG "current scheduler:%d, entry-args [%p-%p]", current_thread->pid, current_thread->entry_point, current_thread->args);
                return current_thread;
            }
        }
    }
    ERR_PRINTK(MODULE_NAME_LOG "cant retrive current scheduler thread\n");

    return NULL;
}

int release_scheduler(struct list_head *sched_data, unsigned long int filep)
{
    int ret;
    //unsigned long flags, flags2;
    //struct task_struct *helper_kill;
    //struct list_head *current_item_list;
    UMS_sched_data *current_UMS, *UMS_temp;
    int check = 0;

    while (1)
    {
        check = 0;
        list_for_each_entry_safe(current_UMS, UMS_temp, sched_data, next)
        {
            //current_UMS = list_entry(current_item_list, UMS_sched_data, next);

            if (current_UMS->id_counter == filep)
            {
                //struct list_head *current_thread_list;
                scheduler_thread *cursor, *temp;

                list_for_each_entry_safe(cursor, temp, &current_UMS->scheduler_thread, next)
                {
                    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "im going to delete and kill:%d\n", cursor->pid);
                    cursor->destroy = 1;
                    if (cursor->nesting > 0 || cursor->currently_running != NULL)
                        check++;
                    if (cursor->currently_running != NULL)
                        DEBUG_DO_PPRINTK(MODULE_NAME_LOG "release scheduler while: cnesting:%d , curently: %p, state:%d,prio:%d, destroy:%d\n", cursor->nesting, cursor->currently_running, cursor->currently_running->state, cursor->yield_priority, cursor->destroy);
                    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "while, cnesting:%d , curently: %p, prio:%d, destroy:%d\n", cursor->nesting, cursor->currently_running, cursor->yield_priority, cursor->destroy);
                }
            }
            //return 0;
        }
        if (check == 0)
            break;
        else
            return -5;
    }

    list_for_each_entry_safe(current_UMS, UMS_temp, sched_data, next)
    {
        //current_UMS = list_entry(current_item_list, UMS_sched_data, next);

        if (current_UMS->id_counter == filep && current_UMS->owner == current->pid)
        {
            //struct list_head *current_thread_list;
            scheduler_thread *cursor, *temp;

            list_for_each_entry_safe(cursor, temp, &current_UMS->scheduler_thread, next)
            {
                DEBUG_DO_PPRINTK(MODULE_NAME_LOG "im going to delete and kill:%d\n", cursor->pid);

                //send signal to thread;

                //if (cursor->task_struct == NULL)
                //    ERR_PRINTK(MODULE_NAME_LOG "NO TASK STRUCT\n");
                //spin_lock_irqsave(&(cursor->completion_list->list_lock), flags);
                //spin_lock_irqsave(&(cursor->sched_lock), flags2);

                while (1)
                {
                    //msleep(2000);
                    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "release scheduler while_2: cnesting:%d , currently: %p, prio:%d, destroy:%d,cur_scheduler->started:%d,cur_scheduler->ended:%d;\n", cursor->nesting, cursor->currently_running, cursor->yield_priority, cursor->destroy, cursor->started, cursor->ended);

                    if (cursor->nesting <= 0 && cursor->currently_running == NULL)
                    {
                        //DEBUG_DO_PPRINTK(MODULE_NAME_LOG "madno dsegnale a :%d\n", cursor->pid);
                        cursor->destroy = 1;
                        //send_sig_info(SIG_TEST, &info, cursor->task_struct); //send signal to user land
                        //helper_kill = cursor->task_struct;
                        //spin_unlock_irqrestore(&(cursor->completion_list->list_lock), flags);
                        break;
                    }
                    return -5;
                }

                //send_sig_info(SIG_TEST, &info, cursor->task_struct); //send signal to user land
                //spin_unlock_irqrestore(&(cursor->sched_lock), flags2);
                //spin_unlock_irqrestore(&(cursor->completion_list->list_lock), flags);
                rm_proc_ums_scheduler_thread(filep, cursor);
                list_del(&cursor->next);
                kfree(cursor);
            }

            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "list empty?: %d\n", list_empty(&current_UMS->scheduler_thread));
            ret = list_empty(&current_UMS->scheduler_thread);
            if (!ret)
            {
                ERR_PPRINTK(MODULE_NAME_LOG "list is not empty ret=%d\n", ret);
                return -1;
            }
            //struct kernel_siginfo info;
            //memset(&info, 0, sizeof(struct kernel_siginfo));
            //info.si_signo = SIG_TEST;
            //info.si_code = SI_QUEUE;
            rm_proc_ums_scheduler(filep, current_UMS);
            list_del(&current_UMS->next);
            //send_sig_info(SIG_TEST, &info, helper_kill); //send signal to user land

            kfree(current_UMS);
            return 0;
        }
    }
    return -1;
}

struct completion_list *retrive_list(struct list_head *comp_lists, ums_pid_t id)
{
    struct list_head *current_item;
    completion_list *current_list;

    int ret = list_empty(comp_lists);
    if (ret)
    {
        ERR_PPRINTK(MODULE_NAME_LOG "list is  empty etrive_list ret=%d\n", ret);
        return NULL;
    }
    list_for_each(current_item, comp_lists)
    {
        current_list = list_entry(current_item, completion_list, next);

        if (current_list->id == id)
        {

            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "list founded [%llu-%llu]\n", current_list->id, id);
            return current_list;
        }
    }
    ERR_PPRINTK(MODULE_NAME_LOG "completion list not found: %llu\n", id);
    return NULL;
}

struct worker_thread *retrive_worker_thread(struct completion_list *current_list, ums_pid_t pid)
{
    struct list_head *current_item_list;
    worker_thread *current_worker;
    list_for_each(current_item_list, &current_list->workers)
    {
        current_worker = list_entry(current_item_list, worker_thread, next);

        if (current_worker->pid == pid)
        {
            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "worker found:%llu\n", current_worker->pid);
            return current_worker;
        }
    }
    NOTIFY_EXEC_ERR_PPRINTK(MODULE_NAME_LOG "worker  not found,likely this worked is just been executed by another scheduler pid: %llu\n", pid);

    return NULL;
}

int list_copy_to_user(struct completion_list_data *to_write, struct completion_list *from_read)
{

    struct ums_worker_thread_data *aux;
    struct ums_worker_thread_data *worker_next;
    struct list_head *worker_head;
    struct worker_thread *current_worker;
    unsigned long flags;

    if (to_write == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "Invalid completion_list_data pointer\n");
        return -1;
    }

    if (to_write->requested_size <= 0)
    {
        ERR_PRINTK(MODULE_NAME_LOG "NO SPACE TO WRITE-1\n");
        return -1;
    }

    aux = kmalloc(sizeof(ums_worker_thread_data), GFP_KERNEL);
    if (aux == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "ERROR IN KMALLOC\n");
        return -1;
    }
    copy_from_user(aux, (ums_worker_thread_data *)to_write->head, sizeof(ums_worker_thread_data));

    if (aux == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "NO SPACE TO WRITE-2\n");
        return -1;
    }

    worker_next = to_write->head;

    list_for_each(worker_head, &(from_read->workers))
    {
        if (to_write->requested_size <= 0)
        {
            DEBUG_DO_PRINTK(MODULE_NAME_LOG "list_copy_to_user no more space-1\n");
            break;
        }

        if (aux == NULL)
        {
            DEBUG_DO_PRINTK(MODULE_NAME_LOG "list_copy_to_user no more space-2\n");
            break;
        }

        current_worker = list_entry(worker_head, worker_thread, next);

        spin_lock_irqsave(&(current_worker->execute_lock), flags);
        if (current_worker == NULL)
        {
            spin_unlock_irqrestore(&(current_worker->execute_lock), flags);
            ERR_PRINTK(MODULE_NAME_LOG "current worker is now null\n");
            break;
        }

        if (current_worker->state != RUNNABLE)
        {

            spin_unlock_irqrestore(&(current_worker->execute_lock), flags);
            continue;
        }

        aux->pid = current_worker->pid;
        aux->completion_list_id = current_worker->completion_list_id;
        aux->work = current_worker->work;
        aux->args = current_worker->args;
        //worker_next=aux.next;
        //aux.next=NULL;

        copy_to_user(worker_next, aux, sizeof(ums_worker_thread_data));
        //aux=aux->next;

        to_write->requested_size--;
        if (to_write->requested_size > 0)
        {
            worker_next = aux->next;
            copy_from_user(aux, (ums_worker_thread_data *)worker_next, sizeof(ums_worker_thread_data));
        }
        spin_unlock_irqrestore(&(current_worker->execute_lock), flags);
    }
    aux->next = NULL;

    copy_to_user(worker_next, aux, sizeof(ums_worker_thread_data));
    kfree(aux);
    return 0;
}

int del_worker_thread(struct list_head worker_list, struct worker_thread *worker_to_del)
{

    //struct list_head *current_worker_list;
    struct worker_thread *cursor, *temp;
    unsigned long flags;
    list_for_each_entry_safe(cursor, temp, &worker_list, next)
    {
        spin_lock_irqsave(&cursor->execute_lock, flags);
        if (cursor == NULL)
        {
            ERR_PRINTK(MODULE_NAME_LOG "CRITIAL ERROR\n");
            spin_unlock_irqrestore(&cursor->execute_lock, flags);
            break;
        }
        if (cursor->pid == worker_to_del->pid)
        {

            list_del(&cursor->next);
            kfree(cursor);
            //ERR_PRINTK("del_worker_thread1\n");
            spin_unlock_irqrestore(&cursor->execute_lock, flags);
            //ERR_PRINTK("del_worker_thread2\n");
            return 0;
        }
        spin_unlock_irqrestore(&cursor->execute_lock, flags);
    }
    ERR_PRINTK(MODULE_NAME_LOG "del_worker_thread\n");
    return -1;
}

char *_retrive_schduler_thread_proc(time64_t pid, struct list_head *sched_data)
{
    struct list_head *current_ums_list;
    struct UMS_sched_data *current_ums;

    struct list_head *current_thread_list;
    struct scheduler_thread *current_thread;
    char *buf_to_write = kmalloc(sizeof(char) * BUFSIZE, GFP_KERNEL);
    unsigned long pid_cur_run = 0;
    if (buf_to_write == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "Error in kmalloc retrive_schduler_thread_proc\n");
        return NULL;
    }

    //struct scheduler_thread aux;

    list_for_each(current_ums_list, sched_data)
    {
        current_ums = list_entry(current_ums_list, UMS_sched_data, next);

        list_for_each(current_thread_list, &current_ums->scheduler_thread)
        {
            current_thread = list_entry(current_thread_list, scheduler_thread, next);
            if (current_thread->pid == pid)
            {

                if (current_thread->currently_running != NULL)
                    pid_cur_run = current_thread->currently_running->pid;
                sprintf(buf_to_write, "#switches:%d\ncompletion_list_id:%llu\nlast_time_for_Switch:%llu\nmean_time_for_switch:%llu\nrunning:%lu\n", current_thread->numb_switch, current_thread->completion_list->id, current_thread->last_switch_time, current_thread->mean_switch_time, pid_cur_run);

                return buf_to_write;
            }
        }
    }

    return NULL;
}

char *_retrive_worker_thread_proc(time64_t pid, struct list_head *comp_lists)
{
    struct list_head *current_list_item, *current_worker_list;
    struct completion_list *current_list;
    struct worker_thread *current_worker;
    struct timespec64 ts2;
    time64_t aux_time_var = 0;
    char *buf_to_write = kmalloc(sizeof(char) * BUFSIZE, GFP_KERNEL);
    if (buf_to_write == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "Error in kmalloc retrive_worker_thread_proc\n");
        return NULL;
    }
    //TODO chewck kmalloc

    //long unsigned flag1, flag2;
    //struct worker_thread *aux = NULL;

    list_for_each(current_list_item, comp_lists)
    {
        current_list = list_entry(current_list_item, completion_list, next);

        list_for_each(current_worker_list, &current_list->workers)
        {
            current_worker = list_entry(current_worker_list, worker_thread, next);

            if (current_worker->pid == pid)
            {

                if (current_worker->runnning_time == 0 && current_worker->starting_time != 0)
                {

                    ktime_get_real_ts64(&ts2);
                    DEBUG_DO_PPRINTK("[%lld-%lld]\n", ts2.tv_sec, current_worker->starting_time);
                    aux_time_var = ts2.tv_sec - current_worker->starting_time;
                }
                else if (current_worker->runnning_time != 0 && current_worker->starting_time == 0)
                {
                    //siamo in un caso di yield
                    aux_time_var = current_worker->runnning_time;
                }
                else if (current_worker->runnning_time == 0 && current_worker->starting_time == 0)
                {

                    aux_time_var = current_worker->runnning_time;
                }
                else
                {

                    aux_time_var = current_worker->runnning_time + ts2.tv_sec - current_worker->starting_time;
                }
                sprintf(buf_to_write, "#switches:%lu\nrunning_time:%lld\nstate:%d\n", current_worker->numb_switch, aux_time_var, current_worker->state);

                return buf_to_write;
            }
        }
    }

    ERR_PPRINTK(MODULE_NAME_LOG "cant retrive the worker for the proc\n");
    return NULL;
}

int destoy_comp_list_set_destroy(unsigned long arg, struct list_head *comp_lists)
{

    unsigned long flags;
    struct completion_list *desired_comp_list;
    struct completion_list_data comp_list_arg;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "destoy_comp_list_set_destroy\n");
    if ((completion_list_data *)arg == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "DESTROY_COMP_LIST no arg passed\n");
        return -EINVAL;
    }
    copy_from_user(&comp_list_arg, (completion_list_data *)arg, sizeof(completion_list_data));

    desired_comp_list = retrive_list(comp_lists, comp_list_arg.id);

    if (desired_comp_list == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "DESTROY_COMP_LIST no completion list found\n");
        return -EINVAL;
    }
    spin_lock_irqsave(&desired_comp_list->list_lock, flags);
    desired_comp_list->to_destroy = 1;
    spin_unlock_irqrestore(&desired_comp_list->list_lock, flags);

    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit: destoy_comp_list_set_destroy\n");
    return 0;
}

int add_worker(unsigned long arg, ums_pid_t worker_counter, struct list_head *comp_lists)
{
    //riceve un puntatore a ums_worker_thread_data

    struct worker_thread *new_worker_thread;
    struct completion_list *desired_comp_list;
    struct ums_worker_thread_data worker_to_add;
    unsigned long flags2;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "add_worker\n");
    if ((ums_worker_thread_data *)arg == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "ERRORE: ADD_WORKER invalid arg\n");
        return -EINVAL;
    }
    copy_from_user(&worker_to_add, (ums_worker_thread_data *)arg, sizeof(ums_worker_thread_data));

    new_worker_thread = kmalloc(sizeof(worker_thread), GFP_KERNEL);
    if (new_worker_thread == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "Error in kmalloc add_worker\n");
        return -EINVAL;
    }

    new_worker_thread->state = RUNNABLE;
    new_worker_thread->work = worker_to_add.work;
    new_worker_thread->args = worker_to_add.args;
    new_worker_thread->completion_list_id = worker_to_add.completion_list_id;
    new_worker_thread->numb_switch = 0;
    new_worker_thread->runnning_time = 0;
    spin_lock_init(&(new_worker_thread->execute_lock));
    new_worker_thread->task_struct = get_pid_task(find_get_pid(worker_to_add.pid), PIDTYPE_PID);
    new_worker_thread->pid = worker_counter;

    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "new_worker_thread->work %p\n", new_worker_thread->work);
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "worker_to_add->work %p\n", worker_to_add.work);
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "new_worker_thread->pid %llu\n", new_worker_thread->pid);

    desired_comp_list = retrive_list(comp_lists, worker_to_add.completion_list_id);

    if (desired_comp_list == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "ERRORE: ADD_WORKER invalid compilation list id\n");

        return -EINVAL;
    }

    spin_lock_irqsave(&desired_comp_list->list_lock, flags2);
    if (unlikely(desired_comp_list->to_destroy == 1))
    {
        ERR_PRINTK(MODULE_NAME_LOG "list to be destroyed\n");
        spin_unlock_irqrestore(&desired_comp_list->list_lock, flags2);

        return -EACCES;
    }
    list_add(&(new_worker_thread->next), &(desired_comp_list->workers));
    spin_unlock_irqrestore(&desired_comp_list->list_lock, flags2);
    //increment available worker threads in a completion list
    up(&(desired_comp_list->sem_counter));

    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "added worker to the comp list:%lu\n", new_worker_thread->completion_list_id);
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit add_worker\n");
    return 0;
}

int exit_worker_thread(struct list_head *sched_data, unsigned long int filep, unsigned long arg)
{

    // Oppure più semplicemente potete imporre che al termine venga chiamata una funzione ExitWorkerThread() che ritorna allo scheduler.

    struct scheduler_thread *cur_scheduler;
    struct sched_thread_data aux;
    unsigned long flags;
    struct scheduler_thread_worker_lifo *current_worker_lifo, *current_worker_lifo_temp;
    struct worker_thread *worker;
    struct timespec64 ts2;
    int check;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit_worker_thread\n");
    cur_scheduler = retrive_current_scheduler_thread(sched_data, filep);

    if (cur_scheduler == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "cant retrive the current_scheduler_thread_info from EXIT_WORKER_THREAD\n");

        return -EINVAL;
    }

    /*
        if(cur_scheduler->currently_running->state==RUNNABLE){
            ERR_PRINTK(MODULE_NAME_LOG "the worker is runnable EXIT_WORKER_THREAD\n");
            spin_unlock_irqrestore(&UMS_lock, flags4);
            return -EINVAL;

        }
        */

    cur_scheduler->currently_running->state = EXIT;

    //qua ritorniamo la famosa scheduling netry_point allo user cosi' da poterla eseguire
    aux.entry_point = cur_scheduler->entry_point;
    aux.args = cur_scheduler->args;
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "sto per retornarti [%p-%p]\n", cur_scheduler->entry_point, cur_scheduler->args);
    copy_to_user((sched_thread_data *)arg, &aux, sizeof(sched_thread_data));

    check = 0; //check fifo or lifo
    //spin_lock_irqsave(&COMPLETION_lock, flags2);
    spin_lock_irqsave(&(cur_scheduler->completion_list->list_lock), flags);
    list_for_each_entry_safe(current_worker_lifo, current_worker_lifo_temp, &cur_scheduler->worker_lifo, next)
    {
        if (cur_scheduler->currently_running == current_worker_lifo->this_worker)
        {

            //del_worker_thread(cur_scheduler->completion_list->workers, cur_scheduler->currently_running);
            cur_scheduler->ended++;
            list_del(&(current_worker_lifo->next));
            kfree(current_worker_lifo);
            worker = cur_scheduler->currently_running;
            rm_proc_worker(worker, cur_scheduler);

            //stoppo il tempo di running del worker
            ktime_get_real_ts64(&ts2);
            cur_scheduler->currently_running->runnning_time += ts2.tv_sec - cur_scheduler->currently_running->starting_time;

            cur_scheduler->currently_running = NULL;

            check++;

            if (del_worker_thread(cur_scheduler->completion_list->workers, worker) < 0)
            {
                ERR_PPRINTK(MODULE_NAME_LOG "cant remove worker from completion list:%d\n", current->pid);
                break;
            }

            //cur_scheduler->currently_running = NULL;
        }
        else
        {
            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "worker_lifo=%p, check: %d\n", current_worker_lifo->this_worker, check);
            cur_scheduler->currently_running = current_worker_lifo->this_worker;
            cur_scheduler->currently_running->state = RUNNING;
            break;
        }
    }
    spin_unlock_irqrestore(&(cur_scheduler->completion_list->list_lock), flags);
    //spin_unlock_irqrestore(&COMPLETION_lock, flags2);
    //context_switch(cur_scheduler->currently_running->task_struct, cur_scheduler->task_struct);
    if (cur_scheduler->nesting > 0)
    {
        if (down_trylock(&(cur_scheduler->completion_list->sem_counter)))
        {
            DEBUG_DO_PPRINTK(MODULE_NAME_LOG "currently_tuning=%p\n", cur_scheduler->currently_running);
            DEBUG_DO_PRINTK(MODULE_NAME_LOG "EXIT_WORKER_THREAD no workers but u have nesting\n");

            return -EBUSY;
        }
        else
        {
            cur_scheduler->yield_priority = PRIO;

            return 0;
        }
    }

    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit exit_worker_thread\n");
    return 0;
}

int exit_from_yield(struct list_head *sched_data, unsigned long int filep)
{
    struct scheduler_thread *cur_scheduler;
    struct timespec64 ts2;
    cur_scheduler = retrive_current_scheduler_thread(sched_data, filep);

    if (cur_scheduler == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "cant retrive the current_scheduler_thread_info from exit_from_yield\n");
        return -EINVAL;
    }
    //il worker torna ad  essere eseguto
    ktime_get_real_ts64(&ts2);
    cur_scheduler->currently_running->starting_time = ts2.tv_sec;

    //il worker riceve un nuovo switch
    cur_scheduler->currently_running->numb_switch++;

    //
    cur_scheduler->numb_switch++;

    //torno indietr nel nesting
    cur_scheduler->nesting--;

    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit:exit_from_yield\n");
    return 0;
}

int ums_thread_yield(struct list_head *sched_data, unsigned long int filep, unsigned long arg)
{
    //called from a worker thread, it pauses the execution of the current thread and the UMS scheduler entry point is executed for determining the next thread to be scheduled;
    struct scheduler_thread *cur_scheduler;
    struct sched_thread_data aux;
    struct timespec64 ts2;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG " ums_thread_yield\n");

    cur_scheduler = retrive_current_scheduler_thread(sched_data, filep);

    if (cur_scheduler == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "cant retrive the current_scheduler_thread_info\n");

        return -EINVAL;
    }

    if (down_trylock(&(cur_scheduler->completion_list->sem_counter)))
    {
        //No workers
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "UMS_THREAD_YIELD no workers\n");
        return -EBUSY;
    }
    else
    {
        DEBUG_DO_PRINTK(MODULE_NAME_LOG "UMS_THREAD_YIELD workers present\n");
        cur_scheduler->nesting++;
        cur_scheduler->yield_priority = PRIO;
        cur_scheduler->currently_running->state = YIELD;
        //cur_scheduler->currently_running->numb_switch++;

        aux.entry_point = cur_scheduler->entry_point;
        aux.args = cur_scheduler->args;

        copy_to_user((sched_thread_data *)arg, &aux, sizeof(sched_thread_data));
        //return -22;

        //stoppo il tempo di running del worker
        ktime_get_real_ts64(&ts2);
        cur_scheduler->currently_running->runnning_time += ts2.tv_sec - cur_scheduler->currently_running->starting_time;
        cur_scheduler->currently_running->starting_time = 0;
        return -EAGAIN;
    }

    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit  ums_thread_yield\n");
    return 0;
}

int _execute_ums_thread(struct list_head *sched_data, unsigned long int filep, unsigned long arg, struct scheduler_thread *cur_scheduler, struct scheduler_thread_worker_lifo *worker_lifo_aux)
{

    struct completion_list *cur_list;
    struct ums_worker_thread_data retrived_worker_to_execute;
    unsigned long flags2, flags;
    struct worker_thread *worker_to_execute;
    struct timespec64 ts2;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG " _execute_ums_thread\n");
    if ((ums_worker_thread_data *)arg == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "ERROR: EXECUTE_UMS_THREAD invalid arg\n");
        return -EINVAL;
    }

    cur_list = cur_scheduler->completion_list;
    if (cur_list == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "No completion list related to this UMS Scheduler thread,EXECUTE_UMS_THREAD\n");

        return -EINVAL;
    }

    copy_from_user(&retrived_worker_to_execute, (ums_worker_thread_data *)arg, sizeof(ums_worker_thread_data));

    spin_lock_irqsave(&cur_list->list_lock, flags2);
    worker_to_execute = retrive_worker_thread(cur_list, retrived_worker_to_execute.pid);

    if (worker_to_execute == NULL)
    {

        NOTIFY_EXEC_ERR_PPRINTK(MODULE_NAME_LOG "ERROR: worker not found arg:%ld\n", arg);
        spin_unlock_irqrestore(&cur_list->list_lock, flags2);

        return -EBADF;
    }

    spin_lock_irqsave(&(worker_to_execute->execute_lock), flags);

    if (worker_to_execute->state != RUNNABLE)
    {

        DEBUG_DO_PPRINTK(MODULE_NAME_LOG "ERROR: EXECUTE_UMS_THREAD you are trying to execute a non runnable worker_thread\n");
        spin_unlock_irqrestore(&worker_to_execute->execute_lock, flags);
        spin_unlock_irqrestore(&cur_list->list_lock, flags2);

        return -EACCES;
    }
    cur_scheduler->currently_running = worker_to_execute;
    cur_scheduler->currently_running->state = RUNNING;
    spin_unlock_irqrestore(&worker_to_execute->execute_lock, flags);
    spin_unlock_irqrestore(&cur_list->list_lock, flags2);

    cur_scheduler->yield_priority = NOT_PRIO;
    cur_scheduler->started++;
    cur_scheduler->numb_switch++;
    //worker_to_schedule.state = RUNNING;
    //copy_to_user(arg, cur_scheduler->currently_running, sizeof(worker_thread));
    //curr_task_struct = get_pid_task(find_get_pid(current->pid), PIDTYPE_PID);

    //qua dobbiamo solo retornare in relata' la worker_thread_data allo user
    retrived_worker_to_execute.work = worker_to_execute->work;
    retrived_worker_to_execute.args = worker_to_execute->args;
    retrived_worker_to_execute.pid = worker_to_execute->pid;
    retrived_worker_to_execute.completion_list_id = worker_to_execute->completion_list_id;
    retrived_worker_to_execute.next = NULL;

    //aumento la var di numb_Swtichs del worker
    worker_to_execute->numb_switch++;
    ktime_get_real_ts64(&ts2);
    worker_to_execute->starting_time = ts2.tv_sec;

    //inserisco il worker nella ia lifo
    worker_lifo_aux->this_worker = worker_to_execute;
    list_add(&(worker_lifo_aux->next), &(cur_scheduler->worker_lifo));
    create_proc_worker(worker_to_execute, cur_scheduler);
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "retrived_worker_to_execute.work %p\n", retrived_worker_to_execute.work);
    DEBUG_DO_PPRINTK(MODULE_NAME_LOG "worker_to_execute->work %p\n", worker_to_execute->work);

    copy_to_user((ums_worker_thread_data *)arg, &retrived_worker_to_execute, sizeof(ums_worker_thread_data));
    //copy_to_user((ums_worker_thread_data*)arg,&retrived_worker_to_execute,sizeof(unsigned long *));
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit _execute_ums_thread\n");
    return 0;
}

int execute_ums_thread(struct list_head *sched_data, unsigned long int filep, unsigned long arg, struct scheduler_thread_worker_lifo *worker_lifo_aux)
{

    struct timespec64 ts;
    struct scheduler_thread *cur_scheduler;
    time64_t start_time;
    time64_t exit_time;
    long start_time_nsec;
    long exit_time_nsec;
    int err;
    int alpha;
    unsigned long int new_value;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "execute_ums_thread\n");
    ktime_get_real_ts64(&ts);
    start_time = ts.tv_sec;
    start_time_nsec = ts.tv_nsec;
    exit_time_nsec = 0;
    exit_time = 0;

    worker_lifo_aux = kmalloc(sizeof(scheduler_thread_worker_lifo), GFP_KERNEL);
    if (worker_lifo_aux == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "ERROR: EXECUTE_UMS_THREAD cant alloc\n");
        return -EFBIG;
    }

    //while this can be done in the execute ums_thread, since we need again this structure for time_data, it's better to call the retrive_Current_scheduler_thread here.
    cur_scheduler = retrive_current_scheduler_thread(sched_data, filep);
    if (cur_scheduler == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "cant retrive  UMS Scheduler thread: EXECUTE_UMS_THREAD\n");
        return -EINVAL;
    }

    err = _execute_ums_thread(sched_data, filep, arg, cur_scheduler, worker_lifo_aux);

    //time mangement, cant be done in a function, should be done before exit, to not compromise the data
    ktime_get_real_ts64(&ts);
    exit_time = ts.tv_sec;
    exit_time_nsec = ts.tv_nsec;
    //accumulator = (alpha * new_value) + (1.0 - alpha) * accumulator
    alpha = 5;

    if (exit_time == start_time)
    {
        new_value = exit_time_nsec - start_time_nsec;
    }
    else
    {
        new_value = (unsigned long int)(exit_time - start_time) * 1000000000;
    }
    cur_scheduler->mean_switch_time = (new_value / alpha) + cur_scheduler->mean_switch_time * (alpha - 1) / alpha;
    cur_scheduler->last_switch_time = new_value;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit execute_ums_thread\n");
    return err;
}

int dequeue_size_request(struct list_head *sched_data, unsigned long int filep, unsigned long arg)
{
    struct completion_list *cur_list;
    struct completion_list_data cld;
    unsigned long flags, flags2;
    struct list_head *current_worker;
    struct worker_thread *worker;
    cur_list = retrive_current_list(sched_data, filep);

    if (cur_list == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "No completion list related to this UMS Scheduler thread\n");

        return -EINVAL;
    }

    cld.id = cur_list->id;
    cld.head = NULL;
    cld.requested_size = 1;

    spin_lock_irqsave(&(cur_list->list_lock), flags2);
    list_for_each(current_worker, &(cur_list->workers))
    {
        worker = list_entry(current_worker, worker_thread, next);
        spin_lock_irqsave(&(worker->execute_lock), flags);
        if ((worker->state) != RUNNABLE)
        {
            spin_unlock_irqrestore(&(worker->execute_lock), flags);

            continue;
        }
        spin_unlock_irqrestore(&(worker->execute_lock), flags);
        cld.requested_size++;
    }

    copy_to_user((completion_list_data *)arg, &cld, sizeof(struct completion_list_data));

    spin_unlock_irqrestore(&(cur_list->list_lock), flags2);
    return 0;
}

int enter_ums_scheduling_mode(struct list_head *sched_data, unsigned long int filep, unsigned long arg, struct list_head *comp_lists)
{
    struct sched_thread_data *ums;
    int err;
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "enter_ums_scheduling_mode\n");
    if (((sched_thread_data *)arg) == NULL)
    {

        ERR_PRINTK(MODULE_NAME_LOG "error UMS_sched_data\n");
        return -EINVAL;
    }

    ums = kmalloc(sizeof(sched_thread_data), GFP_KERNEL);
    if (ums == NULL)
    {
        ERR_PRINTK(MODULE_NAME_LOG "error in kmalloc enter ums scheduling mode\n");
        return -EINVAL;
    }
    err = copy_from_user(ums, (sched_thread_data *)arg, sizeof(sched_thread_data));
    if (err)
        return -EINVAL;

    err = create_ums_thread(sched_data, filep, ums->entry_point, ums->args, comp_lists, ums->completion_list);

    if (err < 0)
    {
        ERR_PRINTK(MODULE_NAME_LOG "ERROR: create_ums_thread\n");
        return -EINVAL;
    }
    DEBUG_DO_PRINTK(MODULE_NAME_LOG "exit:enter_ums_scheduling_mode\n");
    return 0;
}

//endline