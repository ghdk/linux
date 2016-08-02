..
.. Copyright (c) 2016 Dionysios Kalofonos
..
.. Permission is hereby granted, free of charge, to any person obtaining a copy
.. of this software and associated documentation files (the "Software"), to deal
.. in the Software without restriction, including without limitation the rights
.. to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
.. copies of the Software, and to permit persons to whom the Software is
.. furnished to do so, subject to the following conditions:
..
.. The above copyright notice and this permission notice shall be included in
.. all copies or substantial portions of the Software.
..
.. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
.. IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
.. FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
.. AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
.. LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
.. OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
.. SOFTWARE.
..

.. _GNU GRUB Manual 2.00: http://www.gnu.org/software/grub/manual/grub.html
.. _Multiboot 2: http://download-mirror.savannah.gnu.org/releases/grub/phcoder/multiboot.pdf

============
Boot Process
============

Background
==========
According to the  
`Intel Software Developer’s Manual <http://www.intel.co.uk/content/www/uk/en/architecture-and-technology/64-ia-32-architectures-software-developer-manual-325462.html>`_
the first instruction that is fetched and executed following a hardware reset is
located at physical address FFFFFFF0H. At this point conrol is passed to the 
BIOS which runs diagnostic tests and configures the devices of the system. At 
the very end BIOS passes control to the boot loader. When the BIOS supports 
`EFI <http://www.uefi.org/>`_, it loads EFI applications from the EFI System
Partition. If the BIOS is legacy, it loads the boot loader from the Master Boot
Record (MBR).

GRUB - Multiboot
================
The boot loader that we will be using is `GRUB 2 <http://www.gnu.org/software/grub/>`_.
GRUB is modular, it is shipped with a large set of modules and it can be easily 
extended further. It supports both legacy and EFI capable BIOS. GRUB 2 has three
interfaces for loading a kernel or a second boot loader, namely chainloader, 
linux and multiboot.

Chainloading
------------
Chainloading is the method of loading a second boot loader. When GRUB is
loaded by a legacy BIOS from the MBR, it expects that the second boot
loader will be loaded in the same fashion. As such it expects that the
chainloader command will be passed a series of blocks that correspond to the
second boot loader (see the
`block list syntaxt <https://www.gnu.org/software/grub/manual/html_node/Block-list-syntax.html#Block-list-syntax>`_).
For more information please see the source of the
`lecacy loader <http://git.savannah.gnu.org/cgit/grub.git/tree/grub-core/loader/i386/pc/chainloader.c>`_.

When GRUB is loaded from an EFI capable BIOS, it expects that the second
boot loader will also be an EFI application. As such it expects that the
chainloader command will be given a file name of an EFI application to load.
For more information please see the source of the
`EFI loader <http://git.savannah.gnu.org/cgit/grub.git/tree/grub-core/loader/efi/chainloader.c>`_.

Linux
-----
This is the method of loading a Linux kernel. The first argument to the linux
command is the kernel to load, and and subsequent arguments are passed as
arguments to the kernel.

Multiboot
---------
Multiboot is the method of loading a kernel that adheres to the Multiboot
`version 1 <https://www.gnu.org/software/grub/manual/multiboot/multiboot.html>`_ or
`version 2 <http://download-mirror.savannah.gnu.org/releases/grub/phcoder/multiboot.pdf>`_
specifications. For example lets take a minimal kernel program. The _start
function calls main:

.. code-block:: gas
   :linenos:

   .section .text
   .global _start
   .type _start, @function
   _start:
      andl $0xfffffff0, %esp
      call main
   loop:
      hlt
      jmp loop
   .size _start, . - _start

The main program just returns 0xDEADBEEF:

.. code-block:: c
   :linenos:

   #if defined(__cplusplus)
   extern "C"
   #endif
   int main(void)
   {
      return 0xDEADBEEF;
   }

The linker script puts the pieces together:

.. code-block:: text
   :linenos:

   ENTRY(_start)

   SECTIONS
   {
      . = 1M;
      .text :
      {
         *(.text)
         . = ALIGN(8K);
      }
      .data :
      {
         *(.data)
         . = ALIGN(8K);
      }
      .bss :
      {
         *(.bss)
         . = ALIGN(8K);
      }
      .comment :
      {
         *(.comment)
      }
   }

When we boot this kernel we see that GRUB complains about not beeing able to
find the Multiboot header.

.. image:: multiboot.png

Multiboot compliant kernels contain a Multiboot header which
should appear within the first 32768 bytes of the executable. The following
example introduces a .multiboot section, which is split in two subsections. The
first holds the header for the Multiboot 1 specification, and the second
holds the header for the Multiboot 2 specification:

.. code-block:: gas
   :linenos:

   .section .multiboot
   .align 8
   mbAs:
      .long 0x1BADB002            # MAGIC
      .long 0x1                   # FLAGS
      .long 0 - 0x1BADB002 - 0x1  # CHECKSUM
   mbAe:
   .align 8
   mbBs:
      .long 0xE85250D6                          # MAGIC
      .long 0                                   # ARCHITECTURE
      .long mbBe - mbBs                         # HEADER LENGTH
      .long 0 - 0xE85250D6 - 0 - (mbBe - mbBs)  # CHECKSUM
      .short 0                                  # END TAG
      .short 0                                  # TAG FLAG
      .long 8                                   # TAG SIZE
   mbBe:
   .section .text
   .global _start
   .type _start, @function
   _start:
      andl $0xfffffff0, %esp
      call main
   loop:
      hlt
      jmp loop
   .size _start, . - _start

We need to modify our linker script so that the .multiboot section appears at
the beginning of the executable:

.. code-block:: text
   :linenos:

   ENTRY(_start)

   SECTIONS
   {
      . = 1M;
      .multiboot :
      {
         *(.multiboot)
         . = ALIGN(8K);
      }
      .text :
      {
         *(.text)
         . = ALIGN(8K);
      }
      .data :
      {
         *(.data)
         . = ALIGN(8K);
      }
      .bss :
      {
         *(.bss)
         . = ALIGN(8K);
      }
      .comment :
      {
         *(.comment)
      }
   }

.. eof
