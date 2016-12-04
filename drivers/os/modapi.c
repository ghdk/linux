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
#include <linux/moduleparam.h>  // module_param
#include <linux/fs.h>           // struct file_operators
#include <linux/export.h>       // THIS_MODULE

#define MODULE_NAME "modapi"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dionysios Kalofonos");
MODULE_DESCRIPTION("Poke the Module API");

char *device = "modapi";
module_param(device, charp, 0);

struct file_operations fops = {
    .owner = THIS_MODULE
};

static int __init mod_init(void)
{
    printk(KERN_INFO MODULE_NAME ": %d: %s\n", __LINE__, __func__);
    printk(KERN_INFO MODULE_NAME ": using device: %s\n", device);
    return 0;
}

module_init(mod_init);

static void __exit mod_exit(void)
{
    printk(KERN_INFO MODULE_NAME ": %d: %s\n", __LINE__, __func__);
}

module_exit(mod_exit);
