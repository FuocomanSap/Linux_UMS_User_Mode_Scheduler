#ifndef ums_H_DEF
#define ums_H_DEF


#include <linux/slab.h>
#include<linux/init.h>

//#include <asm/ptrace.h>
//#include <linux/kernel.h>  
//#include <linux/fs.h>
#include <linux/module.h>  
//#include <linux/uaccess.h> 
#include <linux/device.h>  
//#include <asm/uaccess.h> 
//#include <asm/ioctl.h>
//#include <linux/sched.h>
//#include <linux/list.h>
#include <linux/kfifo.h>
//#include <linux/spinlock.h>
//#include <linux/hrtimer.h>
//#include <linux/completion.h> 
//#include <asm/processor.h>
//#include <linux/processor.h>
/*
#include <linux/init.h>
#include <linux/mutex.h>   
#include <linux/errno.h>   
#include <linux/timer.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/time.h>

*/

//#include "../Headers/def_struct.h"
//#include "../Headers/Common.h"
#include "module_library.h"

//#include "../Headers/user_struct.h"

/**
 * @file ums.h
 * @brief Contains the functions to interact from user space with the kernel module
*/





/**
 * @brief Used to create a new UMS scheduler
 * 
 * @param inodep 
 * @param filep 
 * @return int 0 on success, err otherwise
 */
int ums_dev_open(struct inode *inodep, struct file *filep);

/**
 * @brief Function not implemented, to close call the ioctl(DESTROY_COMP_LIST) and then ioctl(RELEASE_UMS)
 * 
 * @param inodep 
 * @param filep 
 * @return int 0 on success, err otherwise
 */
int ums_dev_release(struct inode *inodep, struct file *filep);

/**
 * @brief This function wraps: all the ioctls declared in Common.h
 * 
 * @param filep 
 * @param cmd the IOCTL defined in Common.h
 * @param arg used to copy_from/copy_to user space
 * @return long 
 */
long ums_dev_ioctls(struct file *filep, unsigned int cmd, unsigned long arg);


/*struct worker_thread* retrive_worker_thread_proc(long unsigned pid);
struct scheduler_thread* retrive_schduler_thread_proc(long unsigned pid);
*/

/**
 * @brief retrives the info of a worker thread and puts it in a buffer
 * 
 * @param pid the selected worker id
 * @return char* !=NULL on success, NULL otherwise
 */
char* retrive_worker_thread_proc(time64_t pid);

/**
 * @brief retrive the info of a schduler thread and puts it in a buffer
 * 
 * @param pid the selected schduler thread's id
 * @return char* !=NULL on success, NULL otherwise
 */
char* retrive_schduler_thread_proc(time64_t pid);

#endif