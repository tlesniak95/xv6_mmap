

////////////////////////////////////////////////////////////////////////////////////////////////////

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
USE COMMENT TO KEEP TRACK OF EDITS


#### Changelog
mmap.h      |  Created file




#### Notes

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
    allocates a  new page table with no user mappings with setupkvm
    allocates a memoryfor each segment with allocuvm
    and loads each segment into memory with loaduvm
        loaduvm uses walkpgdir to find the physical address of the allocated memory at which to write each page
    of the ELF segment and readi to read from the file
    

