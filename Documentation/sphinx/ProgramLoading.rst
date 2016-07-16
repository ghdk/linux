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

.. _ELF: http://www.skyfree.org/linux/references/ELF_Format.pdf
.. _Linux x86 Program Start Up: http://dbp-consulting.com/tutorials/debugging/linuxProgramStartup.html


===============
Program loading
===============

Introduction
============
Let us start the discussion by writing a hello world program

.. code-block:: c
   :linenos:

   #include <stdio.h>

   int main(int argc, char **argv)
   {
       printf("Hello world!\n");
       return 0;
   }

Using the compiler of our distribution we can build this program as expected

.. code-block:: console
   :linenos:

   # gcc main.c
   # ./a.out
   Hello world!

However, if we try to build the same program with our cross compilers we get

.. code-block:: console
   :linenos:

   # /opt/cross-i686/bin/i686-elf-gcc main.c
   main.c:1:19: fatal error: stdio.h: No such file or directory
   #include <stdio.h>
                     ^
   compilation terminated.

Many parts of the C standard library rely on having an operating system. Since
we are writing an operating system, we get only a small subset of the C
standard library, which can be found in the include subdirectory of our
cross compiler. 

If we strip away all the code that relies on the standard library we end up

.. code-block:: c
   :linenos:

   int main(void)
   {
       return 0;
   }

If we try to compile this, we get 

.. code-block:: console
   :linenos:

   # /opt/cross-i686/bin/i686-elf-gcc main.c -nostdlib -ffreestanding
   /opt/cross-i686/lib/gcc/i686-elf/4.9.3/../../../../i686-elf/bin/ld: warning: cannot find entry symbol _start; defaulting to 08048054

This says that we do not have a C runtime either. Since our cross compiler is
built without targeting a specific OS, it does not know which loader will be
used to execute our program.

ELF: Executable and Linking Format
==================================
Before we discuss in detail the C runtime, and how a program is loaded for
execution, we need to have a look at the 
`ELF`_ format. `ELF`_ is 
the format we chose for our kernel executable, by building our cross-compilers 
with the target

.. code-block:: console
   :linenos:

   ~ # export TARGET=i686-elf
   
Let us compile and study the executable of the program

.. code-block:: c
   :linenos:

   int main(void)
   {
       return 0;
   }

.. code-block:: console
   :linenos:

   # gcc main.c
   # readelf -a a.out
   ELF Header:
     Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
     Class:                             ELF32
     Data:                              2's complement, little endian
     Version:                           1 (current)
     OS/ABI:                            UNIX - System V
     ABI Version:                       0
     Type:                              EXEC (Executable file)
     Machine:                           Intel 80386
     Version:                           0x1
     Entry point address:               0x80482d0
     Start of program headers:          52 (bytes into file)
     Start of section headers:          3584 (bytes into file)
     Flags:                             0x0
     Size of this header:               52 (bytes)
     Size of program headers:           32 (bytes)
     Number of program headers:         8
     Size of section headers:           40 (bytes)
     Number of section headers:         30
     Section header string table index: 27

:Magic:
   The first four bytes of the file hold a magic number identifying the 
   file as an `ELF`_ object file, ie. 0x7f, 0x45 = E, 0x4c = L, 0x46 = F.

:Entry point address:
   The address of the _start function of the program. This is the first function
   that is being run during program execution. In the next chapter we discuss it
   in detail.

:Flags:
   Flags associated with the file. For 32 bit files this is always zero.

.. code-block:: console
   :linenos:

   Section Headers:
     [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
     [ 0]                   NULL            00000000 000000 000000 00      0   0  0
     [ 1] .interp           PROGBITS        08048134 000134 000013 00   A  0   0  1
     [ 2] .note.ABI-tag     NOTE            08048148 000148 000020 00   A  0   0  4
     [ 3] .note.gnu.build-i NOTE            08048168 000168 000024 00   A  0   0  4
     [ 4] .gnu.hash         GNU_HASH        0804818c 00018c 000020 04   A  5   0  4
     [ 5] .dynsym           DYNSYM          080481ac 0001ac 000040 10   A  6   1  4
     [ 6] .dynstr           STRTAB          080481ec 0001ec 000045 00   A  0   0  1
     [ 7] .gnu.version      VERSYM          08048232 000232 000008 02   A  5   0  2
     [ 8] .gnu.version_r    VERNEED         0804823c 00023c 000020 00   A  6   1  4
     [ 9] .rel.dyn          REL             0804825c 00025c 000008 08   A  5   0  4
     [10] .rel.plt          REL             08048264 000264 000010 08  AI  5  12  4
     [11] .init             PROGBITS        08048274 000274 000023 00  AX  0   0  4
     [12] .plt              PROGBITS        080482a0 0002a0 000030 04  AX  0   0 16
     [13] .text             PROGBITS        080482d0 0002d0 000182 00  AX  0   0 16
     [14] .fini             PROGBITS        08048454 000454 000014 00  AX  0   0  4
     [15] .rodata           PROGBITS        08048468 000468 000008 00   A  0   0  4
     [16] .eh_frame_hdr     PROGBITS        08048470 000470 00002c 00   A  0   0  4
     [17] .eh_frame         PROGBITS        0804849c 00049c 0000b0 00   A  0   0  4
     [18] .init_array       INIT_ARRAY      0804954c 00054c 000004 00  WA  0   0  4
     [19] .fini_array       FINI_ARRAY      08049550 000550 000004 00  WA  0   0  4
     [20] .jcr              PROGBITS        08049554 000554 000004 00  WA  0   0  4
     [21] .dynamic          DYNAMIC         08049558 000558 0000e8 08  WA  6   0  4
     [22] .got              PROGBITS        08049640 000640 000004 04  WA  0   0  4
     [23] .got.plt          PROGBITS        08049644 000644 000014 04  WA  0   0  4
     [24] .data             PROGBITS        08049658 000658 000008 00  WA  0   0  4
     [25] .bss              NOBITS          08049660 000660 000004 00  WA  0   0  1
     [26] .comment          PROGBITS        00000000 000660 000039 01  MS  0   0  1
     [27] .shstrtab         STRTAB          00000000 000699 000106 00      0   0  1
     [28] .symtab           SYMTAB          00000000 0007a0 000420 10     29  45  4
     [29] .strtab           STRTAB          00000000 000bc0 00023f 00      0   0  1
   Key to Flags:
     W (write), A (alloc), X (execute), M (merge), S (strings)
     I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)
     O (extra OS processing required) o (OS specific), p (processor specific)

:.init:
   This section holds initialisation routines, which are executed before the
   main program entry point (ie. main function for C programs). This section is
   populated by the linker, according to the target OS, and it can be customised
   with a linker script.
   
:.fini:
   This section holds termination routines, which are executed when a program 
   terminates. This section is populated by the linker, according to the target
   OS, and it can be customised with a linker script.

:.text:
   This section holds the executable instructions of a program.

:.data:
   This section holds initialised variables. 

:.bss:
   This section holds uninitialised variables, which are initialised to zero
   when the program starts executing.

:.rodata:
   This section holds initialised readonly variables.

:.plt:
   This section holds the procedure linkage table.

.. code-block:: console
   :linenos:
   
   Program Headers:
     Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
     PHDR           0x000034 0x08048034 0x08048034 0x00100 0x00100 R E 0x4
     INTERP         0x000134 0x08048134 0x08048134 0x00013 0x00013 R   0x1
         [Requesting program interpreter: /lib/ld-linux.so.2]
     LOAD           0x000000 0x08048000 0x08048000 0x0054c 0x0054c R E 0x1000
     LOAD           0x00054c 0x0804954c 0x0804954c 0x00114 0x00118 RW  0x1000
     DYNAMIC        0x000558 0x08049558 0x08049558 0x000e8 0x000e8 RW  0x4
     NOTE           0x000148 0x08048148 0x08048148 0x00044 0x00044 R   0x4
     GNU_EH_FRAME   0x000470 0x08048470 0x08048470 0x0002c 0x0002c R   0x4
     GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10
   
    Section to Segment mapping:
     Segment Sections...
      00     
      01     .interp 
      02     .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr 
             .gnu.version .gnu.version_r .rel.dyn .rel.plt .init .plt .text 
             .fini .rodata .eh_frame_hdr .eh_frame 
      03     .init_array .fini_array .jcr .dynamic .got .got.plt .data .bss 
      04     .dynamic 
      05     .note.ABI-tag .note.gnu.build-id 
      06     .eh_frame_hdr 
      07     

To quote the `ELF`_ standard:

"An executable or shared object file's program header table is an array of 
structures, each describing a segment or other information the system needs to 
prepare the program for execution.  An object file segment contains one or more
sections."

.. code-block:: console
   :linenos:
   
   Dynamic section at offset 0x558 contains 24 entries:
     Tag        Type                         Name/Value
    0x00000001 (NEEDED)                     Shared library: [libc.so.6]
    0x0000000c (INIT)                       0x8048274
    0x0000000d (FINI)                       0x8048454
    0x00000019 (INIT_ARRAY)                 0x804954c
    0x0000001b (INIT_ARRAYSZ)               4 (bytes)
    0x0000001a (FINI_ARRAY)                 0x8049550
    0x0000001c (FINI_ARRAYSZ)               4 (bytes)
    0x6ffffef5 (GNU_HASH)                   0x804818c
    0x00000005 (STRTAB)                     0x80481ec
    0x00000006 (SYMTAB)                     0x80481ac
    0x0000000a (STRSZ)                      69 (bytes)
    0x0000000b (SYMENT)                     16 (bytes)
    0x00000015 (DEBUG)                      0x0
    0x00000003 (PLTGOT)                     0x8049644
    0x00000002 (PLTRELSZ)                   16 (bytes)
    0x00000014 (PLTREL)                     REL
    0x00000017 (JMPREL)                     0x8048264
    0x00000011 (REL)                        0x804825c
    0x00000012 (RELSZ)                      8 (bytes)
    0x00000013 (RELENT)                     8 (bytes)
    0x6ffffffe (VERNEED)                    0x804823c
    0x6fffffff (VERNEEDNUM)                 1
    0x6ffffff0 (VERSYM)                     0x8048232
    0x00000000 (NULL)                       0x0

:.dynamic:
   This section holds dynamic linking information. Dynamic linking (see the 
   `ELF`_ standard, part 2), takes place during program execution. During the
   exec() system call, control is passed to an interpreter who is responsible
   for reading the executable's segments into memory.

.. code-block:: console
   :linenos:
   
   Relocation section '.rel.dyn' at offset 0x25c contains 1 entries:
    Offset     Info    Type            Sym.Value  Sym. Name
   08049640  00000106 R_386_GLOB_DAT    00000000   __gmon_start__
   
   Relocation section '.rel.plt' at offset 0x264 contains 2 entries:
    Offset     Info    Type            Sym.Value  Sym. Name
   08049650  00000107 R_386_JUMP_SLOT   00000000   __gmon_start__
   08049654  00000207 R_386_JUMP_SLOT   00000000   __libc_start_main
   
:.rel.dyn:
   This section holds relocation information for the .dynamic section.

:.rel.plt:
   This section holds relocation information for the .plt section.
   
   From the `ELF`_ standard:
   
   "Relocation is the process of connecting symbolic references with symbolic 
   definitions.  For example, when a program calls a function, the associated 
   call instruction must transfer control to the proper destination address at 
   execution.  In other words, relocatable files must have information that 
   describes how to modify their section contents, thus allowing executable and 
   shared object files to hold the right information for a process's program 
   image."
 
.. code-block:: console
   :linenos:

   Symbol table '.dynsym' contains 4 entries:
      Num:    Value  Size Type    Bind   Vis      Ndx Name
        0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
        1: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
        2: 00000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@GLIBC_2.0 (2)
        3: 0804846c     4 OBJECT  GLOBAL DEFAULT   15 _IO_stdin_used

:.dynsym:
   This section holds the dynamic linking symbol table.
   
.. code-block:: console
   :linenos:
   
   Symbol table '.symtab' contains 66 entries:
      Num:    Value  Size Type    Bind   Vis      Ndx Name
        0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
        1: 08048134     0 SECTION LOCAL  DEFAULT    1 
        2: 08048148     0 SECTION LOCAL  DEFAULT    2 
        3: 08048168     0 SECTION LOCAL  DEFAULT    3 
        4: 0804818c     0 SECTION LOCAL  DEFAULT    4 
        5: 080481ac     0 SECTION LOCAL  DEFAULT    5 
        6: 080481ec     0 SECTION LOCAL  DEFAULT    6 
        7: 08048232     0 SECTION LOCAL  DEFAULT    7 
        8: 0804823c     0 SECTION LOCAL  DEFAULT    8 
        9: 0804825c     0 SECTION LOCAL  DEFAULT    9 
       10: 08048264     0 SECTION LOCAL  DEFAULT   10 
       11: 08048274     0 SECTION LOCAL  DEFAULT   11 
       12: 080482a0     0 SECTION LOCAL  DEFAULT   12 
       13: 080482d0     0 SECTION LOCAL  DEFAULT   13 
       14: 08048454     0 SECTION LOCAL  DEFAULT   14 
       15: 08048468     0 SECTION LOCAL  DEFAULT   15 
       16: 08048470     0 SECTION LOCAL  DEFAULT   16 
       17: 0804849c     0 SECTION LOCAL  DEFAULT   17 
       18: 0804954c     0 SECTION LOCAL  DEFAULT   18 
       19: 08049550     0 SECTION LOCAL  DEFAULT   19 
       20: 08049554     0 SECTION LOCAL  DEFAULT   20 
       21: 08049558     0 SECTION LOCAL  DEFAULT   21 
       22: 08049640     0 SECTION LOCAL  DEFAULT   22 
       23: 08049644     0 SECTION LOCAL  DEFAULT   23 
       24: 08049658     0 SECTION LOCAL  DEFAULT   24 
       25: 08049660     0 SECTION LOCAL  DEFAULT   25 
       26: 00000000     0 SECTION LOCAL  DEFAULT   26 
       27: 00000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
       28: 08049554     0 OBJECT  LOCAL  DEFAULT   20 __JCR_LIST__
       29: 08048310     0 FUNC    LOCAL  DEFAULT   13 deregister_tm_clones
       30: 08048340     0 FUNC    LOCAL  DEFAULT   13 register_tm_clones
       31: 08048380     0 FUNC    LOCAL  DEFAULT   13 __do_global_dtors_aux
       32: 08049660     1 OBJECT  LOCAL  DEFAULT   25 completed.6279
       33: 08049550     0 OBJECT  LOCAL  DEFAULT   19 __do_global_dtors_aux_fin
       34: 080483a0     0 FUNC    LOCAL  DEFAULT   13 frame_dummy
       35: 0804954c     0 OBJECT  LOCAL  DEFAULT   18 __frame_dummy_init_array_
       36: 00000000     0 FILE    LOCAL  DEFAULT  ABS main.c
       37: 00000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c
       38: 08048548     0 OBJECT  LOCAL  DEFAULT   17 __FRAME_END__
       39: 08049554     0 OBJECT  LOCAL  DEFAULT   20 __JCR_END__
       40: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
       41: 08049550     0 NOTYPE  LOCAL  DEFAULT   18 __init_array_end
       42: 08049558     0 OBJECT  LOCAL  DEFAULT   21 _DYNAMIC
       43: 0804954c     0 NOTYPE  LOCAL  DEFAULT   18 __init_array_start
       44: 08049644     0 OBJECT  LOCAL  DEFAULT   23 _GLOBAL_OFFSET_TABLE_
       45: 08048450     2 FUNC    GLOBAL DEFAULT   13 __libc_csu_fini
       46: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_deregisterTMCloneTab
       47: 08048300     4 FUNC    GLOBAL HIDDEN    13 __x86.get_pc_thunk.bx
       48: 08049658     0 NOTYPE  WEAK   DEFAULT   24 data_start
       49: 08049660     0 NOTYPE  GLOBAL DEFAULT   24 _edata
       50: 08048454     0 FUNC    GLOBAL DEFAULT   14 _fini
       51: 08049658     0 NOTYPE  GLOBAL DEFAULT   24 __data_start
       52: 00000000     0 NOTYPE  WEAK   DEFAULT  UND __gmon_start__
       53: 0804965c     0 OBJECT  GLOBAL HIDDEN    24 __dso_handle
       54: 0804846c     4 OBJECT  GLOBAL DEFAULT   15 _IO_stdin_used
       55: 00000000     0 FUNC    GLOBAL DEFAULT  UND __libc_start_main@@GLIBC_
       56: 080483e0    97 FUNC    GLOBAL DEFAULT   13 __libc_csu_init
       57: 08049664     0 NOTYPE  GLOBAL DEFAULT   25 _end
       58: 080482d0     0 FUNC    GLOBAL DEFAULT   13 _start
       59: 08048468     4 OBJECT  GLOBAL DEFAULT   15 _fp_hw
       60: 08049660     0 NOTYPE  GLOBAL DEFAULT   25 __bss_start
       61: 080483cb    10 FUNC    GLOBAL DEFAULT   13 main
       62: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _Jv_RegisterClasses
       63: 08049660     0 OBJECT  GLOBAL HIDDEN    24 __TMC_END__
       64: 00000000     0 NOTYPE  WEAK   DEFAULT  UND _ITM_registerTMCloneTable
       65: 08048274     0 FUNC    GLOBAL DEFAULT   11 _init

:.symtab:
   This section holds a symbol table. We can see in this table our main.c file,
   and our main function at address 080483cb. Try compiling the same program
   with -static, and see how the symbol table changes.

Program linking and loading
===========================
Let us now study the linking and loading processes. The program we will
study is the following

.. code-block:: c
   :linenos:

   int main(void)
   {
       return 0;
   }

If we try to compile this with the compiler that comes with our distribution,
the program compiles cleanly. However, if we use our cross-compilers to compile
this program we get errors

.. code-block:: console
   :linenos:
   
   # i686-elf-gcc main.c
   ld: cannot find crt0.o: No such file or directory
   ld: cannot find -lc
   collect2: error: ld returned 1 exit status

In this example we asked our cross-compiler to link against the C standard 
library, but the linker could not find the crt0.o file. What is the
crt0.o file?

.. code-block:: console
   :linenos:
   
   # i686-elf-gcc -nostdlib -ffreestanding main.c
   ld: warning: cannot find entry symbol _start; defaulting to 08048054

In this example we asked our cross-compiler to not use the C standard library,
but the linker could not find the _start symbol. What is the _start symbol? 

If we have a look at the ELF again, we see in the header the field
'Entry point address'

.. code-block:: console
   :linenos:

   # readelf -h a.out | grep 'Entry'
     Entry point address:               0x80482d0

Let us dissassemble our program and focus on the .text section, which is the
section that holds the executable instructions of a program

.. code-block:: objdump
   :linenos:

   # objdump -S a.out
   a.out:     file format elf32-i386
   
   
   Disassembly of section .plt:
   
   [...]
   
   080482c0 <__libc_start_main@plt>:
    80482c0:   ff 25 54 96 04 08       jmp    *0x8049654
    80482c6:   68 08 00 00 00          push   $0x8
    80482cb:   e9 d0 ff ff ff          jmp    80482a0 <_init+0x2c>

   Disassembly of section .text:
   
   080482d0 <_start>:
    80482d0:   31 ed                   xor    %ebp,%ebp
    80482d2:   5e                      pop    %esi
    80482d3:   89 e1                   mov    %esp,%ecx
    80482d5:   83 e4 f0                and    $0xfffffff0,%esp
    80482d8:   50                      push   %eax
    80482d9:   54                      push   %esp
    80482da:   52                      push   %edx
    80482db:   68 50 84 04 08          push   $0x8048450
    80482e0:   68 e0 83 04 08          push   $0x80483e0
    80482e5:   51                      push   %ecx
    80482e6:   56                      push   %esi
    80482e7:   68 cb 83 04 08          push   $0x80483cb
    80482ec:   e8 cf ff ff ff          call   80482c0 <__libc_start_main@plt>
    80482f1:   f4                      hlt    
    80482f2:   66 90                   xchg   %ax,%ax
    80482f4:   66 90                   xchg   %ax,%ax
    80482f6:   66 90                   xchg   %ax,%ax
    80482f8:   66 90                   xchg   %ax,%ax
    80482fa:   66 90                   xchg   %ax,%ax
    80482fc:   66 90                   xchg   %ax,%ax
    80482fe:   66 90                   xchg   %ax,%ax
   
   08048300 <__x86.get_pc_thunk.bx>:
    8048300:   8b 1c 24                mov    (%esp),%ebx
    8048303:   c3                      ret    
    8048304:   66 90                   xchg   %ax,%ax
    8048306:   66 90                   xchg   %ax,%ax
    8048308:   66 90                   xchg   %ax,%ax
    804830a:   66 90                   xchg   %ax,%ax
    804830c:   66 90                   xchg   %ax,%ax
    804830e:   66 90                   xchg   %ax,%ax
   
   08048310 <deregister_tm_clones>:
    8048310:   b8 63 96 04 08          mov    $0x8049663,%eax
    8048315:   2d 60 96 04 08          sub    $0x8049660,%eax
    804831a:   83 f8 06                cmp    $0x6,%eax
    804831d:   76 1a                   jbe    8048339 <deregister_tm_clones+0x29>
    804831f:   b8 00 00 00 00          mov    $0x0,%eax
    8048324:   85 c0                   test   %eax,%eax
    8048326:   74 11                   je     8048339 <deregister_tm_clones+0x29>
    8048328:   55                      push   %ebp
    8048329:   89 e5                   mov    %esp,%ebp
    804832b:   83 ec 14                sub    $0x14,%esp
    804832e:   68 60 96 04 08          push   $0x8049660
    8048333:   ff d0                   call   *%eax
    8048335:   83 c4 10                add    $0x10,%esp
    8048338:   c9                      leave  
    8048339:   f3 c3                   repz ret 
    804833b:   90                      nop
    804833c:   8d 74 26 00             lea    0x0(%esi,%eiz,1),%esi
   
   08048340 <register_tm_clones>:
    8048340:   b8 60 96 04 08          mov    $0x8049660,%eax
    8048345:   2d 60 96 04 08          sub    $0x8049660,%eax
    804834a:   c1 f8 02                sar    $0x2,%eax
    804834d:   89 c2                   mov    %eax,%edx
    804834f:   c1 ea 1f                shr    $0x1f,%edx
    8048352:   01 d0                   add    %edx,%eax
    8048354:   d1 f8                   sar    %eax
    8048356:   74 1b                   je     8048373 <register_tm_clones+0x33>
    8048358:   ba 00 00 00 00          mov    $0x0,%edx
    804835d:   85 d2                   test   %edx,%edx
    804835f:   74 12                   je     8048373 <register_tm_clones+0x33>
    8048361:   55                      push   %ebp
    8048362:   89 e5                   mov    %esp,%ebp
    8048364:   83 ec 10                sub    $0x10,%esp
    8048367:   50                      push   %eax
    8048368:   68 60 96 04 08          push   $0x8049660
    804836d:   ff d2                   call   *%edx
    804836f:   83 c4 10                add    $0x10,%esp
    8048372:   c9                      leave  
    8048373:   f3 c3                   repz ret 
    8048375:   8d 74 26 00             lea    0x0(%esi,%eiz,1),%esi
    8048379:   8d bc 27 00 00 00 00    lea    0x0(%edi,%eiz,1),%edi
   
   08048380 <__do_global_dtors_aux>:
    8048380:   80 3d 60 96 04 08 00    cmpb   $0x0,0x8049660
    8048387:   75 13                   jne    804839c <__do_global_dtors_aux+0x1c>
    8048389:   55                      push   %ebp
    804838a:   89 e5                   mov    %esp,%ebp
    804838c:   83 ec 08                sub    $0x8,%esp
    804838f:   e8 7c ff ff ff          call   8048310 <deregister_tm_clones>
    8048394:   c6 05 60 96 04 08 01    movb   $0x1,0x8049660
    804839b:   c9                      leave  
    804839c:   f3 c3                   repz ret 
    804839e:   66 90                   xchg   %ax,%ax
   
   080483a0 <frame_dummy>:
    80483a0:   b8 54 95 04 08          mov    $0x8049554,%eax
    80483a5:   8b 10                   mov    (%eax),%edx
    80483a7:   85 d2                   test   %edx,%edx
    80483a9:   75 05                   jne    80483b0 <frame_dummy+0x10>
    80483ab:   eb 93                   jmp    8048340 <register_tm_clones>
    80483ad:   8d 76 00                lea    0x0(%esi),%esi
    80483b0:   ba 00 00 00 00          mov    $0x0,%edx
    80483b5:   85 d2                   test   %edx,%edx
    80483b7:   74 f2                   je     80483ab <frame_dummy+0xb>
    80483b9:   55                      push   %ebp
    80483ba:   89 e5                   mov    %esp,%ebp
    80483bc:   83 ec 14                sub    $0x14,%esp
    80483bf:   50                      push   %eax
    80483c0:   ff d2                   call   *%edx
    80483c2:   83 c4 10                add    $0x10,%esp
    80483c5:   c9                      leave  
    80483c6:   e9 75 ff ff ff          jmp    8048340 <register_tm_clones>
   
   080483cb <main>:
    80483cb:   55                      push   %ebp
    80483cc:   89 e5                   mov    %esp,%ebp
    80483ce:   b8 00 00 00 00          mov    $0x0,%eax
    80483d3:   5d                      pop    %ebp
    80483d4:   c3                      ret    
    80483d5:   66 90                   xchg   %ax,%ax
    80483d7:   66 90                   xchg   %ax,%ax
    80483d9:   66 90                   xchg   %ax,%ax
    80483db:   66 90                   xchg   %ax,%ax
    80483dd:   66 90                   xchg   %ax,%ax
    80483df:   90                      nop
   
   080483e0 <__libc_csu_init>:
    80483e0:   55                      push   %ebp
    80483e1:   57                      push   %edi
    80483e2:   31 ff                   xor    %edi,%edi
    80483e4:   56                      push   %esi
    80483e5:   53                      push   %ebx
    80483e6:   e8 15 ff ff ff          call   8048300 <__x86.get_pc_thunk.bx>
    80483eb:   81 c3 59 12 00 00       add    $0x1259,%ebx
    80483f1:   83 ec 1c                sub    $0x1c,%esp
    80483f4:   8b 6c 24 30             mov    0x30(%esp),%ebp
    80483f8:   8d b3 0c ff ff ff       lea    -0xf4(%ebx),%esi
    80483fe:   e8 71 fe ff ff          call   8048274 <_init>
    8048403:   8d 83 08 ff ff ff       lea    -0xf8(%ebx),%eax
    8048409:   29 c6                   sub    %eax,%esi
    804840b:   c1 fe 02                sar    $0x2,%esi
    804840e:   85 f6                   test   %esi,%esi
    8048410:   74 27                   je     8048439 <__libc_csu_init+0x59>
    8048412:   8d b6 00 00 00 00       lea    0x0(%esi),%esi
    8048418:   8b 44 24 38             mov    0x38(%esp),%eax
    804841c:   89 2c 24                mov    %ebp,(%esp)
    804841f:   89 44 24 08             mov    %eax,0x8(%esp)
    8048423:   8b 44 24 34             mov    0x34(%esp),%eax
    8048427:   89 44 24 04             mov    %eax,0x4(%esp)
    804842b:   ff 94 bb 08 ff ff ff    call   *-0xf8(%ebx,%edi,4)
    8048432:   83 c7 01                add    $0x1,%edi
    8048435:   39 f7                   cmp    %esi,%edi
    8048437:   75 df                   jne    8048418 <__libc_csu_init+0x38>
    8048439:   83 c4 1c                add    $0x1c,%esp
    804843c:   5b                      pop    %ebx
    804843d:   5e                      pop    %esi
    804843e:   5f                      pop    %edi
    804843f:   5d                      pop    %ebp
    8048440:   c3                      ret    
    8048441:   eb 0d                   jmp    8048450 <__libc_csu_fini>
    8048443:   90                      nop
    8048444:   90                      nop
    8048445:   90                      nop
    8048446:   90                      nop
    8048447:   90                      nop
    8048448:   90                      nop
    8048449:   90                      nop
    804844a:   90                      nop
    804844b:   90                      nop
    804844c:   90                      nop
    804844d:   90                      nop
    804844e:   90                      nop
    804844f:   90                      nop
   
   08048450 <__libc_csu_fini>:
    8048450:   f3 c3                   repz ret 

Surprisingly, our main function is only a tiny part of the program's .text
section, and is not even the first function being run when the program
is loaded. The entry point address we got by reading the ELF, points to the
_start function. So where does this function come from?

The _start function is part of the C library, and is contained in the crt0.o
file. To quote the
`Gentoo documentation <https://dev.gentoo.org/~vapier/crt.txt>`_:

"On uClibc/glibc systems, this object initializes very early ABI requirements
(like the stack or frame pointer), setting up the argc/argv/env values, and
then passing pointers to the init/fini/main funcs to the internal libc main
which in turn does more general bootstrapping before finally calling the real
main function.

glibc ports call this file 'start.S' while uClibc ports call this crt0.S or
crt1.S (depending on what their gcc expects)."

But before we go through the _start section we need to discuss what happens
when a program is run. A program is run through the execve() system call (see 
the man page for execve). To quote the `Linux x86 Program Start Up`_:

"To summarize, it will set up a stack for you, and push onto it argc, argv, and 
envp. The file descriptions 0, 1, and 2, (stdin, stdout, stderr), are left to 
whatever the shell set them to. The loader does much work for you setting up 
your relocations, and as we'll see much later, calling your preinitializers. 
When everything is ready, control is handed to your program by calling _start()"

So, in detail, the _start section does the following (from the start.S
`source code <https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=sysdeps/i386/start.S;hb=HEAD>`_): 

.. code-block:: objdump
   :linenos:

   080482d0 <_start>:
    /* Clear the frame pointer, to mark the outermost frame. */
    80482d0:   31 ed                   xor    %ebp,%ebp
    /* Put argc into %esi. */
    80482d2:   5e                      pop    %esi
    /* Put argv into %ecx. */
    80482d3:   89 e1                   mov    %esp,%ecx
    /* 16-byte alignment. */
    80482d5:   83 e4 f0                and    $0xfffffff0,%esp
    80482d8:   50                      push   %eax
    80482d9:   54                      push   %esp
    80482da:   52                      push   %edx
    /* Push the address of .fini. */
    80482db:   68 50 84 04 08          push   $0x8048450
    /* Push the address of .init. */
    80482e0:   68 e0 83 04 08          push   $0x80483e0
    /* Push argv. */
    80482e5:   51                      push   %ecx
    /* Push argc. */
    80482e6:   56                      push   %esi
    /* Push the address of the main function. */
    80482e7:   68 cb 83 04 08          push   $0x80483cb
    /* Call the main function through __libc_start_main. */
    80482ec:   e8 cf ff ff ff          call   80482c0 <__libc_start_main@plt>
    80482f1:   f4                      hlt    
    80482f2:   66 90                   xchg   %ax,%ax
    80482f4:   66 90                   xchg   %ax,%ax
    80482f6:   66 90                   xchg   %ax,%ax
    80482f8:   66 90                   xchg   %ax,%ax
    80482fa:   66 90                   xchg   %ax,%ax
    80482fc:   66 90                   xchg   %ax,%ax
    80482fe:   66 90                   xchg   %ax,%ax

The function __libc_start_main lives in glibc source tree in  
`csu/libc-start.c <https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=csu/libc-start.c;hb=HEAD>`_.
The function __libc_csu_init is a constructor, while the function 
__libc_csu_fini is a destructor. These functions live in glibc source tree in
`csu/elf-init.c <https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=csu/elf-init.c;hb=HEAD>`_.
I will not discuss the inner workings of these function, but if you want to know
more please consult the `Linux x86 Program Start Up`_.

So now that we know what crt0.o and _start are, lets revisit the two error
messages that we received when were building with our cross compiler.

.. code-block:: console
   :linenos:

   # i686-elf-gcc main.c
   ld: cannot find crt0.o: No such file or directory
   ld: cannot find -lc
   collect2: error: ld returned 1 exit status

The standard library that comes with the cross compiler is minimal, as the
cross compiler targets an OS uknown to glibc. As such there is no crt0.o file.

.. code-block:: console
   :linenos:
   
   # i686-elf-gcc -nostdlib -ffreestanding main.c
   ld: warning: cannot find entry symbol _start; defaulting to 08048054

In this example we asked the cross compiler to build the program without
using the standard library. Even though it does not attempt to locate the
crt0.o file, it needs the entry point of the program.
Hence, we need to provide our own _start function. A minimal start.S file
is the following

.. code-block:: gas
   :linenos:

   .section .text
   .global _start
   .type _start, @function
   _start:
      andl $0xfffffff0, %esp  # align the stack to a 16-byte boundary
      call main               # call the main function
   loop:
      jmp _start              # go back to _start
   .size _start, . - _start

The most confusing part of this program is the call to "jmp _start". The _start
function cannot return, as there is no frame before _start to continue execution
at. Returning from _start would result in a segmentation fault. If we
were building a program for linux, at this point we would be making a system
call to exit the program. However, this is a standalone program and it cannot
make system calls. So i decided to let it loop forever, even though that might
not be the best approach.

The main program is a minimal CPP program

.. code-block:: c
   :linenos:
   
   #if defined(__cplusplus)
   extern "C"
   #endif
   int main(void)
   {
      return 0;
   }

Since we are using our own start.S script, we need to provide our own linker 
script. A simple linker script is the following

.. code-block:: text
   :linenos:
   
   ENTRY(_start)
   
   SECTIONS
   {
     . = 0x10000;
     .text : { *(.text) }
     . = 0x8000000;
     .data : { *(.data) }
     .bss : { *(.bss) }
   }
   
This script tells the linker that _start is the entry point of the program, and
that the program has the sections .text starting at address 0x10000, .data 
starting at address 0x8000000, and .bss. Lets compile and have a look at the ELF

.. code-block:: console
   :linenos:

   # i686-elf-as start.S -o start.o
   # i686-elf-g++ -c -o main.o main.cpp
   # i686-elf-g++ -T i686.ld -o a.out -ffreestanding -nostdlib start.o main.o
   # readelf -a a.out
   ELF Header:
   Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
   Class:                             ELF32
   Data:                              2's complement, little endian
   Version:                           1 (current)
   OS/ABI:                            UNIX - System V
   ABI Version:                       0
   Type:                              EXEC (Executable file)
   Machine:                           Intel 80386
   Version:                           0x1
   Entry point address:               0x10000
   Start of program headers:          52 (bytes into file)
   Start of section headers:          4424 (bytes into file)
   Flags:                             0x0
   Size of this header:               52 (bytes)
   Size of program headers:           32 (bytes)
   Number of program headers:         1
   Size of section headers:           40 (bytes)
   Number of section headers:         7
   Section header string table index: 4
   
   Section Headers:
   [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
   [ 0]                   NULL            00000000 000000 000000 00      0   0  0
   [ 1] .text             PROGBITS        00010000 001000 000014 00  AX  0   0  1
   [ 2] .eh_frame         PROGBITS        00010014 001014 000038 00   A  0   0  4
   [ 3] .comment          PROGBITS        00000000 00104c 000011 01  MS  0   0  1
   [ 4] .shstrtab         STRTAB          00000000 001113 000034 00      0   0  1
   [ 5] .symtab           SYMTAB          00000000 001060 000090 10      6   7  4
   [ 6] .strtab           STRTAB          00000000 0010f0 000023 00      0   0  1
   Key to Flags:
   W (write), A (alloc), X (execute), M (merge), S (strings)
   I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)
   O (extra OS processing required) o (OS specific), p (processor specific)
   
   There are no section groups in this file.
   
   Program Headers:
   Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
   LOAD           0x001000 0x00010000 0x00010000 0x0004c 0x0004c R E 0x1000
   
   Section to Segment mapping:
   Segment Sections...
   00     .text .eh_frame
   
   There is no dynamic section in this file.
   
   There are no relocations in this file.
   
   The decoding of unwind sections for machine type Intel 80386 is not currently supported.
   
   Symbol table '.symtab' contains 9 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
   0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND
   1: 00010000     0 SECTION LOCAL  DEFAULT    1
   2: 00010014     0 SECTION LOCAL  DEFAULT    2
   3: 00000000     0 SECTION LOCAL  DEFAULT    3
   4: 00000000     0 FILE    LOCAL  DEFAULT  ABS start.o
   5: 00010008     0 NOTYPE  LOCAL  DEFAULT    1 loop
   6: 00000000     0 FILE    LOCAL  DEFAULT  ABS main.cpp
   7: 00010000    10 FUNC    GLOBAL DEFAULT    1 _start
   8: 0001000a    10 FUNC    GLOBAL DEFAULT    1 main
   
   No version information found in this file.

In the output of readelf we see that the entry point is at 0x10000, we see that
the .text section is located at the same address, and in the symbol table we
see all the symbols of our program. However, we dont see the .data and .bss
sections. Well, we do not have any variables, so these sections have been 
dropped. So lets recompile with debug information and lets go through GDB

.. code-block:: console
   :linenos:

   # i686-elf-as -g start.S -o start.o
   # i686-elf-g++ -g -c -o main.o main.cpp
   # i686-elf-g++ -g -T i686.ld -o a.out -ffreestanding -nostdlib start.o main.o
   # gdb ./a.out
   Reading symbols from ./a.out...done.
      (gdb) b _start
   Breakpoint 1 at 0x10000: file start.S, line 5.
      (gdb) run
   Starting program: a.out

      Breakpoint 1, _start () at start.S:5
   5               andl $0xfffffff0, %esp
      (gdb) s
   6               call main
      (gdb) s
   main () at main.cpp:6
      6               return 0;
   (gdb) s
      7       }
   (gdb) s
      _start () at start.S:12
   12              jmp _start
      (gdb) s

   Breakpoint 1, _start () at start.S:5
      5               andl $0xfffffff0, %esp
      (gdb) q

We set a breakpoint at the _start function, and we ran the program. The program
calls the main function, returns from it, and resumes execution at the _start
again.

.. eof
