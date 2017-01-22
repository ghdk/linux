/*
 * Copyright (C) 2016 Dionysios Kalofonos
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>         // __init, __exit, module_init, and module_exit
#include <linux/kernel.h>       // several utilities, includes printk
#include <linux/module.h>       // MODULE_LICENSE, MODULE_AUTHOR, MODULE_DESCRIPTION
#include <linux/export.h>       // THIS_MODULE

// module information
#define MODULE_NAME "modapi"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dionysios Kalofonos");
MODULE_DESCRIPTION("Poke the Module API");

/*
 * Callback for module loading. __init is an attribute meaning that the
 * memory used by this function can be thrown away after initialisation.
 */
static int __init mod_init(void)
{
    printk(KERN_INFO MODULE_NAME ": %d: %s\n", __LINE__, __func__);
    return 0;
}

/*
 * Callback for module unloading. __exit is an attribute meaning that the
 * function can be ignored when compiling for statically linking.
 */
static void __exit mod_exit(void)
{
    printk(KERN_INFO MODULE_NAME ": %d: %s\n", __LINE__, __func__);
}

// export module functions
module_init(mod_init);
module_exit(mod_exit);
