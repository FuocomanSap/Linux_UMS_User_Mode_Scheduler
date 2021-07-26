#include "export_proc_info.h"
static int len, temp;

static ssize_t read_proc(struct file *file, char *ubuf, size_t count, loff_t *ppos)
{
    char *name = kmalloc(sizeof(char) * RETRIVE_NAME_SIZE, GFP_KERNEL);
    char *father_name = kmalloc(sizeof(char) * RETRIVE_NAME_SIZE, GFP_KERNEL);
    char *grand_father_name = kmalloc(sizeof(char) * RETRIVE_NAME_SIZE, GFP_KERNEL);
    char *info_from_ums = kmalloc(sizeof(char) * BUFSIZE, GFP_KERNEL);
    char *retrived_buffer;
    /*
    char name[RETRIVE_NAME_SIZE];
    char father_name[RETRIVE_NAME_SIZE];
    char grand_father_name[RETRIVE_NAME_SIZE];
    */
    //char buf[BUFSIZE];
    char *buf = kmalloc(sizeof(char) * BUFSIZE, GFP_KERNEL);
    int len = 0;
    ssize_t err;
    unsigned long pid;

    //printk(KERN_DEBUG MODULE_NAME_LOG "read: pid->%d, length=%ld, offset=%llu,\n", current->pid, count, *ppos);
    sprintf(name, "%s\0", file->f_path.dentry->d_iname);
    sprintf(father_name, "%s\0", file->f_path.dentry->d_parent->d_iname);
    sprintf(grand_father_name, "%s\0", file->f_path.dentry->d_parent->d_parent->d_iname);
    //ERR_PPRINTK(KERN_DEBUG MODULE_NAME_LOG "name: %s, father:%s, gand_father:%s\n", name, father_name, grand_father_name);

    if (*ppos > 0 || count < BUFSIZE)
    {
        //ERR_PRINTK(MODULE_NAME_LOG "generic error, export_proc_info.c-30\n");
        return 0;
    }

    if (!strcmp(father_name, workers_dir))
    {

        if (kstrtoul(name, 10, &pid))
        {
            ERR_PPRINTK(MODULE_NAME_LOG "error kstroul_export_proc_info_1\n");
            return -EINVAL;
        }
        retrived_buffer = retrive_worker_thread_proc(pid);
        if (retrived_buffer == NULL)
        {
            ERR_PPRINTK(MODULE_NAME_LOG "worker is null export_proc_info_1 \n");
            return -EINVAL;
        }
        len += sprintf(buf, "this is a worker_thread proc file\n%s", retrived_buffer);
    }
    else if (!strcmp(name, scheduler_info))
    {

        if (kstrtoul(father_name, 10, &pid))
        {
            ERR_PPRINTK(MODULE_NAME_LOG "error kstroul_export_proc_info_2\n");
            return -EINVAL;
        }

        retrived_buffer = retrive_schduler_thread_proc(pid);
        if (retrived_buffer == NULL)
        {
            ERR_PPRINTK(MODULE_NAME_LOG "scheduler is null export_proc_info_2 \n");
            return -EINVAL;
        }
        len += sprintf(buf, "this is a scheduler_thread proc file\n%s", retrived_buffer);
    }
    else
    {
        ERR_PPRINTK(MODULE_NAME_LOG "error in /proc/ums\n");
        return -EINVAL;
    }
    if (copy_to_user(ubuf, buf, len))
        return -EFAULT;
    *ppos = len;

    kfree(name);
    kfree(father_name);
    kfree(grand_father_name);
    kfree(buf);
    kfree(retrived_buffer);
    return len;
}

static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
    static char msg[BUFSIZE];
    ERR_PPRINTK(MODULE_NAME_LOG "Sorry, this operation isn't supported.\n");
    return -EINVAL;
    copy_from_user(msg, buf, count);
    len = count;
    temp = len;

    return count;
}

static struct proc_ops pops = {

    .proc_read = read_proc,
    .proc_write = write_proc};

int create_proc_ums(struct proc_dir_entry **ent)
{
    *ent = proc_mkdir(ums_standard_path, NULL);
    if (*ent == NULL)
    {
        ERR_PRINTK("ent is null for create_proc_ums\n");
    }

    return 0;
}
int create_proc_ums_scheduler(unsigned long filep, struct UMS_sched_data *scheduler, struct proc_dir_entry *father)
{
    //char path[PATH_CREATE_SIZE];
    char *path = kmalloc(sizeof(char) * PATH_CREATE_SIZE, GFP_KERNEL);
    if (father == NULL)
    {
        ERR_PRINTK("father is  NULL,create_proc_ums_scheduler\n");
    }
    //1231/scheduelrs

    sprintf(path, "%lu", filep);
    //ERR_PRINTK("il path è:%s\n",path);
    scheduler->ent = proc_mkdir(path, father);

    if (scheduler->ent == NULL)
    {
        ERR_PRINTK("scheduler->ent is null \n");
    }

    scheduler->ent_write = proc_mkdir(schedulers_dir, scheduler->ent);
    if (scheduler->ent_write == NULL)
    {
        ERR_PRINTK("scheduler->ent_write is null \n");
    }

    scheduler->father = father;
    kfree(path);
    return 0;
}
int create_proc_ums_scheduler_thread(unsigned long filep, struct scheduler_thread *sched, struct proc_dir_entry *father)
{

    char *path = kmalloc(sizeof(char) * PATH_CREATE_SIZE, GFP_KERNEL);
    if (father == NULL)
    {
        ERR_PRINTK("father is null\n");
    }

    sprintf(path, "%lu", sched->pid);

    sched->ent = proc_mkdir(path, father);
    if (sched->ent == NULL)
    {
        ERR_PRINTK("sched->ent is null\n");
    }

    sched->ent_write = proc_mkdir(workers_dir, sched->ent);
    if (sched->ent_write == NULL)
    {
        ERR_PRINTK("sched->ent_write is null\n");
    }
    proc_create(scheduler_info, 0777, sched->ent, &pops);

    sched->father = father;
    kfree(path);
    return 0;
}
int create_proc_worker(struct worker_thread *worker, struct scheduler_thread *sched)
{
    //char path[PATH_CREATE_SIZE];
    char *path = kmalloc(sizeof(char) * PATH_CREATE_SIZE, GFP_KERNEL);
    sprintf(path, "%lu", worker->pid);
    if (sched->ent_write == NULL)
    {
        ERR_PRINTK("ent_write is null\n");
        return -1;
    }
    proc_create(path, 0777, sched->ent_write, &pops);
    kfree(path);
    return 0;
}

int rm_proc_ums(void)
{
    remove_proc_entry(ums_standard_path, NULL);
    return 0;
}
int rm_proc_ums_scheduler(unsigned long filep, struct UMS_sched_data *scheduler)
{

    //char path[PATH_CREATE_SIZE];
    char *path = kmalloc(sizeof(char) * PATH_CREATE_SIZE, GFP_KERNEL);
    sprintf(path, "%lu", filep);

    remove_proc_entry(schedulers_dir, scheduler->ent);
    remove_proc_entry(path, scheduler->father);
    kfree(path);

    return 0;
}
int rm_proc_ums_scheduler_thread(unsigned long filep, struct scheduler_thread *sched)
{
    //char path[PATH_CREATE_SIZE];
    char *path = kmalloc(sizeof(char) * PATH_CREATE_SIZE, GFP_KERNEL);
    sprintf(path, "%lu", sched->pid);

    remove_proc_entry(scheduler_info, sched->ent);
    remove_proc_entry(workers_dir, sched->ent);
    remove_proc_entry(path, sched->father);
    kfree(path);
    return 0;
}
int rm_proc_worker(struct worker_thread *worker, struct scheduler_thread *sched)
{

    //char path[PATH_CREATE_SIZE];
    char *path = kmalloc(sizeof(char) * PATH_CREATE_SIZE, GFP_KERNEL);
    sprintf(path, "%lu", worker->pid);
    if (sched->ent_write == NULL)
    {
        ERR_PRINTK("ent_write is null\n");
        return -1;
    }
    remove_proc_entry(path, sched->ent_write);
    kfree(path);
    return 0;
    // proc_mkdir()
}