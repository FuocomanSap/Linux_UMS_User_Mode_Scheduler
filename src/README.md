# AOSV Final Project Sources
_A.Y. 2020/2021_

Author(s): Daniele De Turris (1919828), Francesco Douglas Scotti di Vigoleno (1743635) 

The sources are structured in this way:

## `Headers/` cointains both header files and libraries for both user and kernel space
* Commonn.h: ioctls definition
* def_struct.h: struct used by the LKM
* export_porc_info.h/export_porc_info.c: code used by the LKM to manage /proc/ums
* ums_user_library.h/ums_user_library.c: library to interct with LKM from user space
* user_struct.h: structs definiton used by ums_user_library.h to exchnage infos with the LKM


## `Module/` cointains the code of the LKM and its library
* ums.c/ums.h: LKM code
* module_library.c/module_library.h: support library for LKM

## `./` cointains the test file
* test.c: test file