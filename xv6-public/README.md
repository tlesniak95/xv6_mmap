#### TRACKING OUR WORK

////////////////////////////////////////////////////////////////////////////////////////////////////

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
USE COMMENT TO KEEP TRACK OF EDITS

grep -rl "/////////////////////"

#### Changelog


mmap.h | Created file |tlesniak
vm.c | Commented walkpgdir using ChatGPT |tlesniak
vm.c | Commented mappages using ChatGPT |tlesniak
syscall.c | added mmap and munmap to syscall |tlesniak
syscall.h | added mmap and munmap to syscall |tlesniak
usys.S | added mmap and munmap to syscall |tlesniak
sysfile.c | added mmap and munmap to syscall |tlesniak
user.h | added mmap and munmap to syscall |tlesniak
sysfile.c | Working on mmap |tlesniak | 10/27/2023
user.h | fixed typo, using int in XV6 |tlesniak | 10/27/2023
vm.c | removed static from two methods, so we ca use them outside | arul
proc.h | added mmaps structure & int mmap_count to manage mmaps for each proccess | arul
proc.c | added init for int mmap_count;
sysfile.c | worked on mmap and munmap, working on fixing file_backed for munmap | arul
proc.h | Changed process struct to include a valid bit, and removed int mmap_count
proc.c | Added init for valid bits
sysfile.c | Finished file_backed for mmap and munmap, but fileread uses mem, while filewrite uses addr + i. Need to test and see if this is correct. | arul
trap.c | Added handler for grows_up page faults | arul
#### vm.c

static pte_t *
walkpgdir(pde_t *pgdir, const void \*va, int alloc)

    Used to loacte the page table entry for a given virutal address within a particular page directory.

    walkpgdir is a function that retrieves the page table entry (PTE) for a specified virtual address (va) within a given page directory (pgdir). If the page table for the virtual address doesn't exist and if the alloc flag is set, the function allocates a new page table. It then initializes this page table and updates the page directory to point to it. The function returns the specific PTE corresponding to the provided virtual address.

    We will use this for:
    1. to find the right page table entry for a given virtual address
    2. in lazy loading when an unmapped page is accessed.
        Handling the page faults involves finding the corresponding page table entry using walkpgdir and then loading the required file data into the page
    3. Page table initialization
        When mapping a file into a new area of memory, we will need to allocate and initialize new page tables. With the alloc flag set we can allocate new page tables when required and then ink them to the main page directory

kalloc()
This is used to allocate a single page of physical memory.
in xv6 kalloc fetches a page from the free list.

    kalloc will be used to allocate new memry pages which can then be mapped to virtual addresses as required

kfree()
This is used to release memory that was previously allocated using kalloc.
Will be used for munmap
Since we are modifying the exit() system call, we may need to use this to free the memory of a process that is exiting.

static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)

    mappages is a function that sets up page table entries to map a range of virtual addresses to corresponding physical addresses. Given a page directory, starting virtual address, size of the range, starting physical address, and permissions:

    It calculates the aligned starting and ending virtual addresses.
    Iteratively, for each page in the range:
    Retrieves (or creates) the page table entry for the current virtual address.
    Checks if this address is already mapped, and if so, triggers a panic.
    Maps the virtual address to the corresponding physical address with the provided permissions.
    The function returns 0 on successful mapping, and -1 on failure (e.g., if unable to create a new page table).

#### Notes from xv6 manual

Main calls kvmalloc to create and switch to a page table with the mappings above KERNBASE required for the kernel to run
mappages installs mapping into a page table for a range of virtual addresses corresponding to a range of physical addresses

for each virtual address to be mapped, mappeages calls walkpgdir to find the address of the PTE for that address
it then initializes the PTE to hold the relevant physical address and flags

walkpgdir uses 10 bits of the virtual address to find the page directory entry. If the alloc argument is set, walkpgdir allocates it and puts its physical address in the page directory. Finally it uses the next 10 bits of the virtual address to find the address of the PTE in the page table page.

PHYSICAL MEMORY ALLOCATION

The kernel must must allocate and free physical memory at runtime.

    4096 - byte pages at a time

It keeps track of which pages are free by threading a linked list through the pages themselves.

ALLOCATING
consists of removing a page from the linked list

FREEING
consists of adding the freed page to the list

This free list is protected by a spin lock

P2V - physical address to virtual address

The allocator sometimes treats addresses as integers in order to perfomr airthmeitc on them and sometimes uses addresses as pointers to read and write memory.
USE CASTS

The stack is a single page and contains a guard page under it that is unmapped

sbrk is a system call for a process to shrink or grow its emory.
It is implemented by the growproc function
if n is positive, growproc allocates one or more physical pages and maps them into the process's address space
if n is negative, growproc unmaps one or more pages from the process's address space amd frees the corresponding physical pages

A processes page table is stored in memory and so the kernel can update the table.
uses allocuvm and deallocuvm

XV6 must invalidate the cached entries. It invalidates stale cached entries by reloading cr3, the register that holds the address of the current page table

Exec creates the user part of an address space.
allocates a new page table with no user mappings with setupkvm
allocates a memoryfor each segment with allocuvm
and loads each segment into memory with loaduvm
loaduvm uses walkpgdir to find the physical address of the allocated memory at which to write each page
of the ELF segment and readi to read from the file



11/7/2023
ALL TESTS PASSING 



starting build

**********************************************************************

Test build
build xv6 using make

**********************************************************************
Trying to build project using make
make xv6.img fs.img
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie -fno-pic -O -nostdinc -I. -c bootmain.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie -fno-pic -nostdinc -I. -c bootasm.S
ld -m    elf_i386 -N -e start -Ttext 0x7C00 -o bootblock.o bootasm.o bootmain.o
objdump -S bootblock.o > bootblock.asm
objcopy -S -O binary -j .text bootblock.o bootblock
./sign.pl bootblock
boot block is 451 bytes (max 510)
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o bio.o bio.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o console.o console.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o exec.o exec.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o file.o file.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o fs.o fs.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o ide.o ide.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o ioapic.o ioapic.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o kalloc.o kalloc.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o kbd.o kbd.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o lapic.o lapic.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o log.o log.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o main.o main.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o mp.o mp.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o picirq.o picirq.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o pipe.o pipe.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o proc.o proc.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o sleeplock.o sleeplock.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o spinlock.o spinlock.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o string.o string.c
gcc -m32 -gdwarf-2 -Wa,-divide   -c -o swtch.o swtch.S
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o syscall.o syscall.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o sysfile.o sysfile.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o sysproc.o sysproc.c
gcc -m32 -gdwarf-2 -Wa,-divide   -c -o trapasm.o trapasm.S
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o trap.o trap.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o uart.o uart.c
./vectors.pl > vectors.S
gcc -m32 -gdwarf-2 -Wa,-divide   -c -o vectors.o vectors.S
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o vm.o vm.c
gcc -m32 -gdwarf-2 -Wa,-divide   -c -o entry.o entry.S
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie -fno-pic -nostdinc -I. -c entryother.S
ld -m    elf_i386 -N -e start -Ttext 0x7000 -o bootblockother.o entryother.o
objcopy -S -O binary -j .text bootblockother.o entryother
objdump -S bootblockother.o > entryother.asm
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie -nostdinc -I. -c initcode.S
ld -m    elf_i386 -N -e start -Ttext 0 -o initcode.out initcode.o
objcopy -S -O binary initcode.out initcode
objdump -S initcode.o > initcode.asm
ld -m    elf_i386 -T kernel.ld -o kernel entry.o bio.o console.o exec.o file.o fs.o ide.o ioapic.o kalloc.o kbd.o lapic.o log.o main.o mp.o picirq.o pipe.o proc.o sleeplock.o spinlock.o string.o swtch.o syscall.o sysfile.o sysproc.o trapasm.o trap.o uart.o vectors.o vm.o  -b binary initcode entryother
objdump -S kernel > kernel.asm
objdump -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > kernel.sym
dd if=/dev/zero of=xv6.img count=10000
10000+0 records in
10000+0 records out
5120000 bytes (5.1 MB, 4.9 MiB) copied, 0.0333165 s, 154 MB/s
dd if=bootblock of=xv6.img conv=notrunc
1+0 records in
1+0 records out
512 bytes copied, 0.000272194 s, 1.9 MB/s
dd if=kernel of=xv6.img seek=1 conv=notrunc
419+1 records in
419+1 records out
214776 bytes (215 kB, 210 KiB) copied, 0.00152351 s, 141 MB/s
gcc -Werror -Wall -o mkfs mkfs.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o ulib.o ulib.c
gcc -m32 -gdwarf-2 -Wa,-divide   -c -o usys.o usys.S
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o printf.o printf.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o umalloc.o umalloc.c
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o cat.o cat.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _cat cat.o ulib.o usys.o printf.o umalloc.o
objdump -S _cat > cat.asm
objdump -t _cat | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > cat.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o echo.o echo.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _echo echo.o ulib.o usys.o printf.o umalloc.o
objdump -S _echo > echo.asm
objdump -t _echo | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > echo.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o forktest.o forktest.c
# forktest has less library code linked in - needs to be small
# in order to be able to max out the proc table.
ld -m    elf_i386 -N -e main -Ttext 0 -o _forktest forktest.o ulib.o usys.o
objdump -S _forktest > forktest.asm
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o grep.o grep.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _grep grep.o ulib.o usys.o printf.o umalloc.o
objdump -S _grep > grep.asm
objdump -t _grep | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > grep.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o init.o init.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _init init.o ulib.o usys.o printf.o umalloc.o
objdump -S _init > init.asm
objdump -t _init | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > init.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o kill.o kill.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _kill kill.o ulib.o usys.o printf.o umalloc.o
objdump -S _kill > kill.asm
objdump -t _kill | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > kill.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o ln.o ln.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _ln ln.o ulib.o usys.o printf.o umalloc.o
objdump -S _ln > ln.asm
objdump -t _ln | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > ln.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o ls.o ls.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _ls ls.o ulib.o usys.o printf.o umalloc.o
objdump -S _ls > ls.asm
objdump -t _ls | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > ls.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o mkdir.o mkdir.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _mkdir mkdir.o ulib.o usys.o printf.o umalloc.o
objdump -S _mkdir > mkdir.asm
objdump -t _mkdir | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > mkdir.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o rm.o rm.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _rm rm.o ulib.o usys.o printf.o umalloc.o
objdump -S _rm > rm.asm
objdump -t _rm | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > rm.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o sh.o sh.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _sh sh.o ulib.o usys.o printf.o umalloc.o
objdump -S _sh > sh.asm
objdump -t _sh | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > sh.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o stressfs.o stressfs.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _stressfs stressfs.o ulib.o usys.o printf.o umalloc.o
objdump -S _stressfs > stressfs.asm
objdump -t _stressfs | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > stressfs.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o usertests.o usertests.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _usertests usertests.o ulib.o usys.o printf.o umalloc.o
objdump -S _usertests > usertests.asm
objdump -t _usertests | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > usertests.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o wc.o wc.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _wc wc.o ulib.o usys.o printf.o umalloc.o
objdump -S _wc > wc.asm
objdump -t _wc | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > wc.sym
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o zombie.o zombie.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _zombie zombie.o ulib.o usys.o printf.o umalloc.o
objdump -S _zombie > zombie.asm
objdump -t _zombie | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > zombie.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 644 blocks have been allocated
balloc: write bitmap block at sector 58


test build PASSED
 (build xv6 using make)

starting test_1

**********************************************************************

Test test_1
Simple mmap with MAP_ANON | MAP_FIXED

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_1.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 674 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS


test test_1 PASSED (1 of 1)
 (Simple mmap with MAP_ANON | MAP_FIXED)

starting test_2

**********************************************************************

Test test_2
Simple mmap/munmap with MAP_ANON | MAP_FIXED

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_2.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 674 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_2 PASSED (1 of 1)
 (Simple mmap/munmap with MAP_ANON | MAP_FIXED)

starting test_3

**********************************************************************

Test test_3
Access the mmap memory allocated with MAP_ANON | MAP_FIXED, then munmap

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_3.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 674 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_3 PASSED (1 of 1)
 (Access the mmap memory allocated with MAP_ANON | MAP_FIXED, then munmap)

starting test_4

**********************************************************************

Test test_4
Try mmap MAP_ANON without MAP_FIXED

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_4.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 674 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_4 PASSED (1 of 1)
 (Try mmap MAP_ANON without MAP_FIXED)

starting test_5

**********************************************************************

Test test_5
Try to allocate memory with MAP_FIXED at an illegal address

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_5.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 674 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_5 PASSED (1 of 1)
 (Try to allocate memory with MAP_FIXED at an illegal address)

starting test_6

**********************************************************************

Test test_6
mmap a file

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_6.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 677 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk...

xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_6 PASSED (1 of 1)
 (mmap a file)

starting test_7

**********************************************************************

Test test_7
Changes to the mmapped memory should be reflected in file after munmap

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_7.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 679 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_7 PASSED (1 of 1)
 (Changes to the mmapped memory should be reflected in file after munmap)

starting test_8

**********************************************************************

Test test_8
MAP_GROWSUP that adds a single page to anonymous mapping

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_8.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 677 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_8 PASSED (1 of 1)
 (MAP_GROWSUP that adds a single page to anonymous mapping)

starting test_9

**********************************************************************

Test test_9
MAP_GROWSUP with file-backed mapping

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_9.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 679 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_9 PASSED (1 of 1)
 (MAP_GROWSUP with file-backed mapping)

starting test_10

**********************************************************************

Test test_10
Try growing memory without MAP_GROWSUP - should segfault

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_10.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 676 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
Segmentation Fault


test test_10 PASSED (1 of 1)
 (Try growing memory without MAP_GROWSUP - should segfault)

starting test_11

**********************************************************************

Test test_11
Two MAP_GROWSUP mappings with a single guard page in between - the lower should not extend

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_11.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 676 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
Segmentation Fault


test test_11 PASSED (1 of 1)
 (Two MAP_GROWSUP mappings with a single guard page in between - the lower should not extend)

starting test_12

**********************************************************************

Test test_12
Child access to parent mmap

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_12.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 678 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
pte_parent: dfbc007
MMAP	 SUCCESS

test test_12 PASSED (1 of 1)
 (Child access to parent mmap)

starting test_13

**********************************************************************

Test test_13
Changes made to mmapped memory should not be reflected in file when MAP_PRIVATE is set

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_13.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 679 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_13 PASSED (1 of 1)
 (Changes made to mmapped memory should not be reflected in file when MAP_PRIVATE is set)

starting test_14

**********************************************************************

Test test_14
Child should access file data with MAP_PRIVATE flag

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_14.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 678 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS

test test_14 PASSED (1 of 1)
 (Child should access file data with MAP_PRIVATE flag)

starting test_15

**********************************************************************

Test test_15
Parent should not see child mofications to memory with MAP_PRIVATE

**********************************************************************
Running xv6 user progam /home/cs537-1/tests/P5/ctests/test_15.c
Trying to build project using make
make xv6.img fs.img
/tmp/tmpbo46gulo/p/Makefile
/tmp/tmpbo46gulo/p/Makefile.test
make: 'xv6.img' is up to date.
make: 'fs.img' is up to date.
make qemu-nox CPUS=1
gcc -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer -fno-stack-protector -fno-pie -no-pie   -c -o tester.o tester.c
ld -m    elf_i386 -N -e main -Ttext 0 -o _tester tester.o ulib.o usys.o printf.o umalloc.o
objdump -S _tester > tester.asm
objdump -t _tester | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$/d' > tester.sym
./mkfs fs.img README _cat _echo _forktest _grep _init _kill _ln _ls _tester _mkdir _rm _sh _stressfs _usertests _wc _zombie 
nmeta 59 (boot, super, log blocks 30 inode blocks 26, bitmap blocks 1) blocks 941 total 1000
balloc: first 679 blocks have been allocated
balloc: write bitmap block at sector 58
qemu-system-i386 -nographic -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp 1 -m 512 
c[?7l[2J[0mSeaBIOS (version 1.15.0-1)



iPXE (https://ipxe.org) 00:03.0 CA00 PCI2.10 PnP PMM+1FF8B4A0+1FECB4A0 CA00

Press Ctrl-B to configure iPXE (PCI 00:03.0)...
                                                                               



Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ tester
tester
MMAP	 SUCCESS


test test_15 PASSED (1 of 1)
 (Parent should not see child mofications to memory with MAP_PRIVATE)

**********************************************************************
Summary:
test build PASSED
 (build xv6 using make)

test test_1 PASSED (1 of 1)
 (Simple mmap with MAP_ANON | MAP_FIXED)

test test_2 PASSED (1 of 1)
 (Simple mmap/munmap with MAP_ANON | MAP_FIXED)

test test_3 PASSED (1 of 1)
 (Access the mmap memory allocated with MAP_ANON | MAP_FIXED, then munmap)

test test_4 PASSED (1 of 1)
 (Try mmap MAP_ANON without MAP_FIXED)

test test_5 PASSED (1 of 1)
 (Try to allocate memory with MAP_FIXED at an illegal address)

test test_6 PASSED (1 of 1)
 (mmap a file)

test test_7 PASSED (1 of 1)
 (Changes to the mmapped memory should be reflected in file after munmap)

test test_8 PASSED (1 of 1)
 (MAP_GROWSUP that adds a single page to anonymous mapping)

test test_9 PASSED (1 of 1)
 (MAP_GROWSUP with file-backed mapping)

test test_10 PASSED (1 of 1)
 (Try growing memory without MAP_GROWSUP - should segfault)

test test_11 PASSED (1 of 1)
 (Two MAP_GROWSUP mappings with a single guard page in between - the lower should not extend)

test test_12 PASSED (1 of 1)
 (Child access to parent mmap)

test test_13 PASSED (1 of 1)
 (Changes made to mmapped memory should not be reflected in file when MAP_PRIVATE is set)

test test_14 PASSED (1 of 1)
 (Child should access file data with MAP_PRIVATE flag)

test test_15 PASSED (1 of 1)
 (Parent should not see child mofications to memory with MAP_PRIVATE)

Passed 16 of 16 tests.
Overall 16 of 16
Points 15 of 15
