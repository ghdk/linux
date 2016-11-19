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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dionysios Kalofonos");
MODULE_DESCRIPTION("Poke the Module API");

char *basename(char *n)
{
    char *i = NULL;
    for(i = n; NULL != i && '\0' != *i; i++)
        if('/' == *i)
            n = i + 1;
    return n;
}

static int __init mod_init(void)
{
    printk(KERN_INFO "%s: %d: %s\n", basename(__FILE__), __LINE__, __func__);
    return 0;
}

module_init(mod_init);

static void __exit mod_exit(void)
{
    printk(KERN_INFO "%s: %d: %s\n", basename(__FILE__), __LINE__, __func__);
}

module_exit(mod_exit);
