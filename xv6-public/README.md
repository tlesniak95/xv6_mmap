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
