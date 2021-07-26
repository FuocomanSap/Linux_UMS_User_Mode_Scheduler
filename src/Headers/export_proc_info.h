#ifndef EXPORT_PROC_INFO_H_DEF
#define EXPORT_PROC_INFO_H_DEF
#include "def_struct.h"
#include <linux/proc_fs.h>
#include "Common.h"
#include "../Module/ums.h"
#include <linux/string.h>
//#include <linux/proc/generic.h>
#define ums_standard_path "ums"
#define schedulers_dir "schedulers"
#define workers_dir "workers"
#define scheduler_info "info"
#define scheduler_standard_path "ums/%lu"
#define scheduler_thread_standard_path "ums/%lu/scheduler/%lu"
#define worker_standard_path "ums/%lu/scheduler/%lu/workers/%lu"

#define PATH_CREATE_SIZE 1024
#define PATH_RM_SIZE 2048
#define RETRIVE_NAME_SIZE 2048
#define WORKER 1
#define SCHEDULER 2
#define SCHEDULER_THREAD 3

#define BUFSIZE 4096

/**
 * 
 * @file export_proc_info.h
 * @brief This file containts, macros and functions used to manage the /proc files 
 * 
 */

/**
 * @brief Create an ums dir in /proc
 * 
 * @param ent double pointer to a proc_dir_entry that will recive the new pointer to the dir
 * @return int 0 on success, -1 otherwise
 */
int create_proc_ums(struct proc_dir_entry **ent);

/**
 * @brief Create a <filep> dir under /proc/ums, and /proc/ums/<filep>/shedulers, <filep> is the identifier of an UMS Scheduler
 * 
 * @param filep id of the UMS Scheduler
 * @param scheduler pointer to the UMS_Sched_data that will recive in the [ent,ent_write,father] the proc_dir_entry pointers.
 * @param father pointer to the father proc_dir_entry
 * @return int 0 on success, err otherwise
 */
int create_proc_ums_scheduler(unsigned long filep,struct UMS_sched_data *scheduler,struct proc_dir_entry *father);

/**
 * @brief Create: a <pid> dir under /proc/ums/<filep>/schedulers,file "info" and a "workers" dir under /proc/ums/<filep>/schedulers/<pid>.  <pid> is the identifier of an UMS Scheduler thread
 * 
 * @param filep id of the UMS Scheduler
 * @param sched pointer to the scheduler_thread that will recive in the [ent,ent_write,father] the proc_dir_entry pointers.
 * @param father pointer to the father proc_dir_entry
 * @return int 0 on success, err otherwise
 */
int create_proc_ums_scheduler_thread(unsigned long filep,struct scheduler_thread *sched,struct proc_dir_entry *father);

/**
 * @brief Create a <pid> file under /proc/ums/<filep>/schedulers/<father_pid>/workers
 * 
 * @param worker pointer to a worker_thread struct to retrive the worker infos
 * @param sched pointer to the scheduler thread who manage this worker
 * @return int 0 on success, err otherwise
 */
int create_proc_worker(struct worker_thread* worker ,struct scheduler_thread* sched);

/**
 * @brief Remove the "ums" dir under /proc
 * 
 * @return int 0 on success, err otherwise
 */
int rm_proc_ums(void);

/**
 * @brief remove the <filep> dir under /proc/ums
 * 
 * @param filep id of the UMS Scheduler
 * @param scheduler pointer to the UMS Scheduler
 * @return int 0 on success, err otherwise
 */
int rm_proc_ums_scheduler(unsigned long filep,struct UMS_sched_data *scheduler);

/**
 * @brief remove the <pid> dir under /proc/ums/<filep>/schedulers
 * 
 * @param filep id of the UMS Scheduler who manage this Ums scheduler thread
 * @param sched pointer to the UMS scheduler thread
 * @return int 0 on success, err otherwise
 */
int rm_proc_ums_scheduler_thread(unsigned long filep,struct scheduler_thread* sched);

/**
 * @brief remove the <pid> file under /proc/ums/<filep>/schedulers/<father-pid>/workers
 * 
 * @param worker pointer to the desired worker to eliminate
 * @param sched pointer to the UMS scheduler thread who manage this worker
 * @return int 0 on success, err otherwise
 */
int rm_proc_worker(struct worker_thread* worker ,struct scheduler_thread* sched);



#endif