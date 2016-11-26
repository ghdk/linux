..
.. Copyright (C) 2016 Dionysios Kalofonos
..
.. This program is free software; you can redistribute it and/or modify
.. it under the terms of the GNU General Public License version 2 as
.. published by the Free Software Foundation.
..

==============
Kernel Modules
==============

Module API
==========
A good starting point for module development is
`The Linux Kernel Module Programming Guide <http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html>`_.

Adding a module to the build system
===================================
Adding a new module to the build system is fairly simple. Let us take for
example the module

.. code-block:: text
   :linenos:

   drivers/os/modapi.c

With this module we are adding to the drivers directory the sub-directory named
"os", and the module modapi.c. In order for the new directory to be considered
during building, we need to append an obj target to the Makefile of the
parent directory:

.. code-block:: console
   :linenos:

   # tail drivers/Makefile
   obj-$(CONFIG_IPACK_BUS)         += ipack/
   obj-$(CONFIG_NTB)               += ntb/
   obj-$(CONFIG_FMC)               += fmc/
   obj-$(CONFIG_POWERCAP)          += powercap/
   obj-$(CONFIG_MCB)               += mcb/
   obj-$(CONFIG_RAS)               += ras/
   obj-$(CONFIG_THUNDERBOLT)       += thunderbolt/
   obj-$(CONFIG_CORESIGHT)         += coresight/
   obj-$(CONFIG_ANDROID)           += android/
   obj-$(CONFIG_OS_MODAPI_C)       += os/

Within the directory "os" we need a makefile, which lists the files that we
need to build for the new obj target:

.. code-block:: console
   :linenos:

   # cat drivers/os/Makefile
   obj-$(CONFIG_OS_MODAPI_C)               += modapi.o

Equivalently, we need to setup the config files. In the Kconfig of the parent
directory we need to source the new Kconfig:

.. code-block:: console
   :linenos:

   # tail drivers/Kconfig

   source "drivers/ras/Kconfig"

   source "drivers/thunderbolt/Kconfig"

   source "drivers/android/Kconfig"

   source "drivers/os/Kconfig"

   endmenu


Within the "os" directory we introduce a new config file, which allows us to
set the value of the variable CONFIG_OS_MODAPI_C. Since we are introducing a
new directory, in our Kconfig we also add a menu entry:

.. code-block:: console
   :linenos:

   # cat drivers/os/Kconfig
   menu "Operating System"

   config OS_MODAPI_C
           tristate "Poke the Module API"
           default n
   endmenu # "Operating System"

We set the type of the variable to tristate as this variable can be set to one
of 'n' for not building the module, 'M' for building the module for dynamic
loading, and '*' for statically linking the module.
