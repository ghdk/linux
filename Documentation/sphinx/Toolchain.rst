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

=========
Toolchain
=========

Building a cross-compiler
=========================
Since I will be building my OS on Linux it is a good idea to prepare and use
a cross compiler. On OSDev wiki there is a very nice 
`article <http://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F>`_ on the 
subject. Moreover, on OSDev wiki there is a   
`tutorial <http://wiki.osdev.org/GCC_Cross-Compiler>`_ on how to prepare a
toolchain for cross compiling an OS. Here I assume you have read the OSDev wiki
pages.

First I set a few environmental variables

.. code-block:: console
   :linenos:

   ~ # export TARGET=i686-elf
   
   ~ # mkdir -p ${HOME}/Applications/cross-i686/bin
   ~ # export PREFIX="${HOME}/Applications/cross-i686"
   ~ # export PATH="${PREFIX}/bin:${PATH}"

Before building GCC, I built binutils 
(see `https://gnu.org/software/binutils/ <https://gnu.org/software/binutils/>`_)

.. code-block:: console
   :linenos:

   ~ # wget http://ftp.gnu.org/gnu/binutils/binutils-2.26.tar.bz2
   ~ # tar xvfj binutils-2.26.tar.bz2
   ~ # mkdir binutils-2.26-build
   ~ # cd binutils-2.26-build
   binutils-2.26-build # ../binutils-2.26/configure --target=${TARGET} 
                                                    --prefix=${PREFIX} 
                                                    --with-sysroot 
                                                    --disable-nls 
                                                    --disable-werror
   binutils-2.26-build # make && make install

It is advisable that when we build a GCC cross compiler to build a version that
is close to the version of the GCC compiler we use to compile. My laptop has 

.. code-block:: console
   :linenos:

   ~ # gcc --version
   gcc (Debian 4.9.2-10) 4.9.2
   
so I downloaded a version based on the 4.9.x release (see 
`https://gcc.gnu.org/ <https://gcc.gnu.org/>`_

.. code-block:: console
   :linenos:

   ~ # wget http://www.netgull.com/gcc/releases/gcc-4.9.3/gcc-4.9.3.tar.gz
   
As advised on the OSDev wiki, I also downloaded the GNU GMP, GNU MPFR, GNU MPC 
and the ISL library

.. code-block:: console
   :linenos:

   gcc-4.9.3 # ./contrib/download_prerequisites

However, the script that came with GCC did not work for me. This is the patch
i applied:

.. code-block:: diff
   :linenos:

   --- contrib/download_prerequisites	2014-02-13 14:06:48.000000000 +0000
   +++ download_prerequisites	2016-07-02 14:25:57.444427985 +0100
   @@ -29,15 +29,17 @@
    GMP=gmp-4.3.2
    MPC=mpc-0.8.1

   -wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$MPFR.tar.bz2 || exit 1
   +URL='http://ftp.vim.org/languages/gcc/infrastructure'
   +
   +wget ${URL}/$MPFR.tar.bz2 || exit 1
    tar xjf $MPFR.tar.bz2 || exit 1
    ln -sf $MPFR mpfr || exit 1

   -wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$GMP.tar.bz2 || exit 1
   +wget ${URL}/$GMP.tar.bz2 || exit 1
    tar xjf $GMP.tar.bz2  || exit 1
    ln -sf $GMP gmp || exit 1

   -wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$MPC.tar.gz || exit 1
   +wget ${URL}/$MPC.tar.gz || exit 1
    tar xzf $MPC.tar.gz || exit 1
    ln -sf $MPC mpc || exit 1

   @@ -46,11 +48,11 @@
      ISL=isl-0.12.2
      CLOOG=cloog-0.18.1

   -  wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$ISL.tar.bz2 || exit 1
   +  wget ${URL}/$ISL.tar.bz2 || exit 1
      tar xjf $ISL.tar.bz2  || exit 1
      ln -sf $ISL isl || exit 1

   -  wget ftp://gcc.gnu.org/pub/gcc/infrastructure/$CLOOG.tar.gz || exit 1
   +  wget ${URL}/$CLOOG.tar.gz || exit 1
      tar xzf $CLOOG.tar.gz || exit 1
      ln -sf $CLOOG cloog || exit 1
    fi

Finally I compiled GCC

.. code-block:: console
   :linenos:

   ~ # mkdir gcc-4.9.3-build
   ~ # cd gcc-4.9.3-build
   gcc-4.9.3-build # ../gcc-4.9.3/configure --target=${TARGET} 
                                            --prefix=${PREFIX} 
                                            --disable-nls 
                                            --enable-languages=c,c++ 
                                            --without-headers
   gcc-4.9.3-build # make all-gcc
   gcc-4.9.3-build # make all-target-libgcc
   gcc-4.9.3-build # make install-gcc
   gcc-4.9.3-build # make install-target-libgcc

I wanted also to prepare a cross compiler for x86_64 so I set the following
environmental variables and repeated the previous steps

.. code-block:: console
   :linenos:

   ~ # export TARGET=x86_64-elf
   
   ~ # mkdir -p ${HOME}/Applications/cross-x86_64/bin
   ~ # export PREFIX="$HOME/Applications/cross-x86_64"
   ~ # export PATH="$PREFIX/bin:$PATH"

Building GRUB
=============
Instructions for downloading GRUB can be found on the GRUB 2
`website <https://www.gnu.org/software/grub/grub-download.html>`_. I cloned
the GRUB repository:

.. code-block:: console
   :linenos:

   # git clone git://git.savannah.gnu.org/grub.git

In the root directory there is an INSTALL file with instructions for compiling
GRUB. I compiled GRUB through the following steps:

.. code-block:: console
   :linenos:

   # ./autogen.sh
   # ./configure
   # make install

From the installation of GRUB we are interested in grub-mkrescue which we will
be using for preparing bootable ISOs, and the lib directory which contains
the modules:

.. code-block:: console
   :linenos:

   # find ./grub2 -iwholename '*mkrescue' -o -iwholename '*lib/grub/*' -prune
   ./grub2/lib/grub/i386-pc
   ./grub2/bin/grub-mkrescue

Building Linux
==============
Linux uses the kbuild framework. We can obtain the list of targets by running

.. code-block:: console
   :linenos:

   # make help

We can specify a build directory in the following fashion

.. code-block:: console
   :linenos:

   # make O=_build ARCH="x86" tinyconfig

In this example the build directory is '_build' and I made the tinyconfig
target, which configures the tiniest possible kernel. An alternative method of
configuring the kernel easily would be to use the default configuration

.. code-block:: console
   :linenos:

   # make O=_build ARCH="x86" defconfig

Having a minimal or default configuration we can run menuconfig to
tweak it

.. code-block:: console
   :linenos:

   # make O=_build ARCH="x86" menuconfig

Once we are happy with the configuration, we can build the kernel. For the x86
architecture I usually make the isoimage target which creates a bootable iso

.. code-block:: console
   :linenos:

   # make O=_build -j2 isoimage

The original isoimage target uses the
`isolinux <http://www.syslinux.org/wiki/index.php?title=ISOLINUX>`_ boot loader.
I replaced isolinux with GRUB, as GRUB is the bootloader that most probably the
reader is familiar with. GRUB provides the command grub-mkrescue which
builds an ISO image:

.. code-block:: console
   :linenos:

   # grub-mkrescue -d ./grub2/lib/grub/i386-pc -o live.iso isoimage

This command is given the directory where the GRUB modules live. We can set
this by running make menuconfig as shown above, and navigating into
Operating System > "GRUB modules path". When I ran grub-mkrescue for the first
time, I got an error message that it could not locate xorriso

.. code-block:: console
   :linenos:

   grub-mkrescue: 323: xorriso: not found

so i had to install the xorriso library. Then xorriso complained that it
could not find the efi.img file

.. code-block:: console
   :linenos:

   xorriso : FAILURE : Cannot find path '/efi.img' in loaded ISO image

so I had to install the mtools package.

The last argument to this command is a directory that will be included in the
ISO. GRUB expects a specific structure

.. code-block:: console
   :linenos:

   # find isoimage
   isoimage/
   isoimage/boot
   isoimage/boot/grub
   isoimage/boot/grub/grub.cfg
   isoimage/boot/bzImage

Our kernel is the file "bzImage". The GRUB configuration file contains

.. code-block:: console
   :linenos:

   # cat isoimage/boot/grub/grub.cfg
   menuentry "Linux" {
      linux /boot/bzImage
      boot
   }

Make sure the open brace is at the same line as the menuentry definition.

As part of this project we are customising the Linux build system so that it
can build our additions. Previously we mentioned the menu
Operating System > "GRUB modules path" where we can declare the path where the
GRUB modules live. The menu Device Drivers > Operating System lists the modules
that we have added and that we can compile as part of Linux.

Building Linux modules
======================
An example module is drivers/os/modapi.c, whose contents are not particularly
interesting at this point. However, it can serve as an example of how a module
can be compiled.

We can run make menuconfig, go to Device Drivers > Operating System and set the
"Poke the Module API" to "M". "M" means that the module will be built for
dynamic loading, while "*" means that the module will be statically linked
against our kernel. Having done so, we can compile all activated modules,
against our source tree with the command:

.. code-block:: console
   :linenos:

   make O=_build ARCH="x86" -j4 modules

The following command does all the above, plus it compiles our module against
the sources of an installed kernel:

.. code-block:: console
   :linenos:

   make -C /lib/modules/3.16.0-4-586/build M=${PWD}/drivers/os CONFIG_OS_MODAPI_C=m modules

Having compiled our module we can copy it to a VM running the corresponding
kernel and try it out:

.. code-block:: console
   :linenos:

   # mkdir -p /lib/modules/3.16.0-4-586/extra
   # mv /tmp/modapi.ko /lib/modules/3.16.0-4-586/extra/
   # depmod /lib/modules/3.16.0-4-586/extra/modapi.ko
   # modprobe modapi
   # lsmod |grep modapi
   modapi                 12386  0
   # modprobe -r modapi
   # tail -n 2 /var/log/messages
   Nov 19 17:42:26 vbdeb kernel: [ 2198.076194] modapi.c: 28: mod_init
   Nov 19 17:42:40 vbdeb kernel: [ 2211.619753] modapi.c: 36: mod_exit

Debugging with a VM
===================
During development I will be using a virtual machine for debugging. One solution
is `QEMU <http://wiki.qemu.org/Main_Page>`_ which has support for
`GDB <http://wiki.qemu.org/Documentation/Debugging>`_ as well as 
`Valgrind <http://wiki.qemu.org/Debugging_with_Valgrind>`_. An alternative is
`VirtualBox <http://www.virtualbox.org/>`_ which has a built in
`debugger <http://www.virtualbox.org/manual/ch12.html#ts_debugger>`_.
Unfortunately on VirtualBox breakpoints do not work when hardware virtualisation
is enabled, which is a requirement for a 64bit VM. The
following is an example of a VirtualBox VM configuration with which debugging
can be used:

.. code-block:: console
   :linenos:

   # VBoxManage showvminfo "OS"
   [...]
   Guest OS:        Other/Unknown
   [...]
   Memory size:     16MB
   Page Fusion:     off
   VRAM size:       16MB
   CPU exec cap:    20%
   HPET:            off
   Chipset:         ich9
   Firmware:        BIOS
   Number of CPUs:  1
   PAE:             off
   Long Mode:       off
   Synthetic CPU:   off
   CPUID overrides: None
   Boot menu mode:  message and menu
   Boot Device (1): DVD
   Boot Device (2): HardDisk
   Boot Device (3): Not Assigned
   Boot Device (4): Not Assigned
   ACPI:            on
   IOAPIC:          on
   Time offset:     0ms
   RTC:             UTC
   Hardw. virt.ext: off
   Nested Paging:   off
   Large Pages:     off
   VT-x VPID:       on
   VT-x unr. exec.: on
   [...]

To enable the debug menu by default we need to set the GUI/Dbg/Enabled property
of the VM:

.. code-block:: console
   :linenos:

   # VBoxManage setextradata "OS" 'GUI/Dbg/Enabled' true
   # VBoxManage getextradata "OS" 'GUI/Dbg/Enabled'
   Value: true

From the debug menu we can launch the console, through which we can set
breakpoints, step through instructions, display the registers and many more.
For example let us suppose we have a kernel with the following main function
at address 0x0010200b:

.. code-block:: c
   :linenos:

   0010200b <main>:
     10200b:       55                      push   %ebp
     10200c:       89 e5                   mov    %esp,%ebp
     10200e:       b8 ef be ad de          mov    $0xdeadbeef,%eax
     102013:       5d                      pop    %ebp
     102014:       c3                      ret

We can set a breakpoint in the VirtualBox console to break the execution at our
main function:

.. code-block:: text
   :linenos:

   VBoxDbg> br 0010200b 1 'echo main'
   Set REM breakpoint 4 at 000000000010200b

When the breakpoint is reached the console shows:

.. code-block:: text
   :linenos:

   VBoxDbg> main
   eax=2badb002 ebx=00010000 ecx=00000000 edx=00000000 esi=00000000 edi=00000000
   eip=0010200b esp=0007fefc ebp=00000000 iopl=0 nv up di pl nz na po nc
   cs=0010 ds=0018 es=0018 fs=0018 gs=0018 ss=0018               eflags=00000006
   0010:0010200b 55                      push ebp

We can press 't' to do a single step:

.. code-block:: text
   :linenos:

   VBoxDbg> t
   VBoxDbg>
   dbgf event: Single step! (rem)
   eax=2badb002 ebx=00010000 ecx=00000000 edx=00000000 esi=00000000 edi=00000000
   eip=0010200c esp=0007fef8 ebp=00000000 iopl=0 nv up di pl nz na po nc
   cs=0010 ds=0018 es=0018 fs=0018 gs=0018 ss=0018               eflags=00000006
   0010:0010200c 89 e5                   mov ebp, esp
   VBoxDbg> t
   VBoxDbg>
   dbgf event: Single step! (rem)
   eax=2badb002 ebx=00010000 ecx=00000000 edx=00000000 esi=00000000 edi=00000000
   eip=0010200e esp=0007fef8 ebp=0007fef8 iopl=0 nv up di pl nz na po nc
   cs=0010 ds=0018 es=0018 fs=0018 gs=0018 ss=0018               eflags=00000006
   0010:0010200e b8 ef be ad de          mov eax, 0deadbeefh
   VBoxDbg> t
   VBoxDbg>
   dbgf event: Single step! (rem)
   eax=deadbeef ebx=00010000 ecx=00000000 edx=00000000 esi=00000000 edi=00000000
   eip=00102013 esp=0007fef8 ebp=0007fef8 iopl=0 nv up di pl nz na po nc
   cs=0010 ds=0018 es=0018 fs=0018 gs=0018 ss=0018               eflags=00000006
   0010:00102013 5d                      pop ebp

We can display the registers with 'r':

.. code-block:: text
   :linenos:

   VBoxDbg> r
   eax=deadbeef ebx=00010000 ecx=00000000 edx=00000000 esi=00000000 edi=00000000
   eip=00102013 esp=0007fef8 ebp=0007fef8 iopl=0 nv up di pl nz na po nc
   cs=0010 ds=0018 es=0018 fs=0018 gs=0018 ss=0018               eflags=00000006
   0010:00102013 5d                      pop ebp

.. eof
