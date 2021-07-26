#!/bin/bash
./unmount.sh
make clean
make
sudo insmod ums_module.ko
sudo chmod 777 /dev/ums_scheduler
sudo dmesg -w
