#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/seq_file.h>

#define PROC_NAME "os_basics_proc_snapshot"

static unsigned int max_tasks = 10;
module_param(max_tasks, uint, 0444);
MODULE_PARM_DESC(max_tasks, "Maximum number of tasks to print from the process list");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manil Bastola");
MODULE_DESCRIPTION("Expose a small scheduler/process snapshot through procfs.");
MODULE_VERSION("1.0");

static int snapshot_show(struct seq_file *seq, void *unused)
{
	struct task_struct *task;
	unsigned int shown = 0;

	seq_printf(seq, "reader_comm: %s\n", current->comm);
	seq_printf(seq, "reader_pid: %d\n", task_pid_nr(current));
	seq_printf(seq, "jiffies: %lu\n\n", jiffies);
	seq_puts(seq, "pid\tcomm\n");

	rcu_read_lock();
	for_each_process(task) {
		if (shown++ >= max_tasks)
			break;

		seq_printf(seq, "%d\t%s\n", task_pid_nr(task), task->comm);
	}
	rcu_read_unlock();

	return 0;
}

static int snapshot_open(struct inode *inode, struct file *file)
{
	return single_open(file, snapshot_show, NULL);
}

static const struct proc_ops snapshot_ops = {
	.proc_open = snapshot_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int __init proc_snapshot_init(void)
{
	if (!proc_create(PROC_NAME, 0444, NULL, &snapshot_ops))
		return -ENOMEM;

	pr_info("os_basics_proc_snapshot: created /proc/%s\n", PROC_NAME);
	return 0;
}

static void __exit proc_snapshot_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	pr_info("os_basics_proc_snapshot: removed /proc/%s\n", PROC_NAME);
}

module_init(proc_snapshot_init);
module_exit(proc_snapshot_exit);
