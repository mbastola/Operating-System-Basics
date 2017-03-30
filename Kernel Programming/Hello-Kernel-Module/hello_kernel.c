#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Manil Bastola");
MODULE_DESCRIPTION("A minimal loadable Linux kernel module for OS basics practice.");
MODULE_VERSION("1.0");

static int __init hello_kernel_init(void)
{
	pr_info("os_basics_hello: loaded on CPU %u\n", raw_smp_processor_id());
	return 0;
}

static void __exit hello_kernel_exit(void)
{
	pr_info("os_basics_hello: unloaded\n");
}

module_init(hello_kernel_init);
module_exit(hello_kernel_exit);
