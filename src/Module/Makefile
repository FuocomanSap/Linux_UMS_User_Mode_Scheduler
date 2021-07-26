KDIR = /lib/modules/$(shell uname -r)/build
obj-m += ums_module.o
ums_module-objs := ums.o module_library.o ../Headers/export_proc_info.o
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean



