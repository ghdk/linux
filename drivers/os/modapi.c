/*
 * Copyright (C) 2016 Dionysios Kalofonos
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#define MODULE_NAME "modapi"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dionysios Kalofonos");
MODULE_DESCRIPTION("Poke the Module API");

char *device = "/dev/foo";
module_param(device, charp, 0);

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
