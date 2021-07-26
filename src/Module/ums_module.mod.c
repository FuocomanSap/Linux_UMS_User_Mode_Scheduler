#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa879d70c, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x4b5317f6, "kmalloc_caches" },
	{ 0xf9a482f9, "msleep" },
	{ 0x81b395b3, "down_interruptible" },
	{ 0x67d54299, "remove_proc_entry" },
	{ 0x7adc5eab, "device_destroy" },
	{ 0x316f58eb, "__register_chrdev" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0x3c80c06c, "kstrtoull" },
	{ 0x813e9f89, "proc_mkdir" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0xebbe12f0, "current_task" },
	{ 0xfc7e2596, "down_trylock" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0x213ef32e, "device_create" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x9b67a295, "kmem_cache_alloc_trace" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0x1c96ba8c, "find_get_pid" },
	{ 0x37a0cba, "kfree" },
	{ 0xcf2a6966, "up" },
	{ 0x111bd0bd, "class_destroy" },
	{ 0xc6bec2d2, "get_pid_task" },
	{ 0x4aba4bc5, "proc_create" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x6c496836, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe914e41e, "strcpy" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "2BFBA9A38352FBDBFBDD30F");
