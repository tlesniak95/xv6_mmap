//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "mmap.h"
#include "memlayout.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// adding mmap.h

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if (argint(n, &fd) < 0)
    return -1;
  if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0)
    return -1;
  if (pfd)
    *pfd = fd;
  if (pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for (fd = 0; fd < NOFILE; fd++)
  {
    if (curproc->ofile[fd] == 0)
    {
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// void *mmap(void* addr, int length, int prot, int flags, int fd, int offset);
void* sys_mmap(void) {
  int int_addr, length, prot, flags, fd, offset;
  struct proc *curproc = myproc();
  //Used argint for address, because argptr does address checking (on pg 45 of the manual), and we want to do it ourselves.  
  if (argint(0, &int_addr) ||
      argint(1, &length) < 0 ||
      argint(2, &prot) < 0 ||
      argint(3, &flags) < 0 ||
      argint(4, &fd) < 0 ||
      argint(5, &offset) < 0) {
    return (void *)-1;
  }
  void* addr = (void *) int_addr;
  
  //Find a spot to put the mmap in the mmap array, in proc.h
  int mmaps_index = -1;
  for(int i = 0; i < MAX_MMAPS; i++) {
    //If the mmap index is not valid, then we can use it.
    if(curproc->mmaps[i].valid == 0) {
      mmaps_index = i;
      break;
    }
  }
  
  if(mmaps_index == -1) {
    cprintf("Can only have 32 mmaps at a time\n");
    return (void *)-1;
  }

  //Find a starting address for the mapping if MAP_FIXED is not set (since we will ignore given address then)
  if (!(flags & MAP_FIXED)) {
    addr = (void *) 0x60000000;
    int valid_address = 0;
    while (addr < (void *)0x80000000) {
      addr +=  PGSIZE; 
      valid_address = 1;
      //Maybe move this addition later, so first allocation is at 0x60000000? Not sure if it matters. 
      for(int i = 0; i < MAX_MMAPS; i++) {
        if(curproc->mmaps[i].valid && addr >= curproc->mmaps[i].start && addr < curproc->mmaps[i].start + curproc->mmaps[i].length) {
          valid_address = 0;
        }
      }

      if (valid_address && (walkpgdir(curproc->pgdir, (void *) addr, 0) == 0)) {
        break;
      }
      
    }
    
    if (addr >= (void *)0x80000000) {
      return (void *)-1;
    }
  } 
  //Otherwise, we use the given address, and check if it is valid
  else {
    // check length    CHECK THIS I am not sure about the 0x20000000. It is just an assumption based on the what is available to us
    if (length <= 0 || length > 0x20000000) {
      return (void *)-1;
    }

    if (addr < (void *)0x60000000 || addr > (void *)0x80000000 || (int) addr % PGSIZE != 0) {
      return (void *)-1; 
    }
  }
  
  if (flags & MAP_GROWSUP) {
    if (length + PGSIZE + (int) addr > KERNBASE) {
      return (void *)-1;
    }
  }
  // Two Main Modes of Operation

  // Mode 1: MAP_ANONYMOUS which is similar to malloc
  // can ignore fd and offset
  if (flags & MAP_ANONYMOUS) {
    //Not sure if cast is correct here
    if ((int) addr + length > KERNBASE) {
      return (void *)-1;
    }
    
    length = PGROUNDUP(length);

    // Implementing similar logic to allocuvm
    //Casted to char * for arithmetic to work.
    for(char * i = (char *) addr; i < length + (char *) addr; i += PGSIZE) {
      void *mem = kalloc();
      if(mem == 0) {
        return (void *)-1;
      }

      if(mappages(curproc->pgdir, (void *)i, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
        kfree(mem);
        return (void *)-1;
      }
    }
    //Set fd to -1 because it is anonymous
    curproc->mmaps[mmaps_index].fd = -1;
  } else {
    // Mode 2: file-backed
    struct file *f = curproc->ofile[fd];
    
    if (!f) {
      return (void *)-1; // Invalid file descriptor.
    }
    
    length = PGROUNDUP(length);
    
    for (int i = 0; i < length; i += PGSIZE) {
      char *mem = kalloc();
      if (mem == 0) {
        return (void *)-1;
      }

      // Read the file contents into the allocated page. Not sure if addr + i is correct here, or mem
      int nread = fileread(f, mem, PGSIZE);
      if (nread < 0) {
        kfree(mem);
        return (void *)-1;
      }
      
      // Now map the page into the process's address space.
      if (mappages(curproc->pgdir, (void *)(addr + i), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
        kfree(mem);
        return (void *)-1;
      }
    }
    //Set fd to the given fd because this is file-backed
    curproc->mmaps[mmaps_index].fd = fd;
  }

  if (flags & MAP_GROWSUP) {
    //Creates a PTE for the guard page, but doesn't map it to a physical page yet, I believe. 
    //kalloc() is called in trap.c, in case of a page fault on the guard page
    walkpgdir(curproc->pgdir, (void *) ((char *) addr + length), 1);
    curproc->mmaps[mmaps_index].has_guard = 1;
  }
  curproc->mmaps[mmaps_index].start = addr;
  curproc->mmaps[mmaps_index].length = length;
  curproc->mmaps[mmaps_index].flags = flags;
  curproc->mmaps[mmaps_index].valid = 1;
  curproc->mmaps[mmaps_index].ref++;
  return (void *) addr;
}

int sys_munmap(void) {

  // int munmap(void* addr, int length);
  struct proc *curproc = myproc();

  //Following same address logic as mmap, with the int_addr
  int int_addr, length;
  int flags = -1;
  int fd = -1; 

  if (argint(0, &int_addr) ||
      argint(1, &length) < 0)
    return -1;

  void *addr = (void *) int_addr;
  if (addr <= (void *)0x60000000 || addr > (void *)0x80000000 || (int) addr % PGSIZE != 0)
    return -1; // Invalid address

  //Finding the mmap matching the given address
  int mmaps_index = -1; 
  for(int i = 0; i < MAX_MMAPS; i++) {
    if(curproc->mmaps[i].valid && curproc->mmaps[i].start == addr) {
      flags = curproc->mmaps[i].flags;
      fd = curproc->mmaps[i].fd;
      mmaps_index = i;
    }
  }

  length = PGROUNDUP(length);
  pte_t *pte;
  int pa;
  //Mode 1: MAP_ANONYMOUS, or File Backed with MAP_PRIVATE, so do not write back to disk
  if(flags & MAP_ANONYMOUS) {
    //Following a similar logic to deallocuvm
    //Casted to char * for arithmetic to work.
    for(char * i = (char *) addr; i < length + (char *) addr; i += PGSIZE) {
      pte = walkpgdir(curproc->pgdir, (void *)i, 0);
      //Move to next PTE if there is nothing for this address
      if(!pte) {
        //Removed logic from deallocuvm here, maybe change back. I think it is just an optimization.
        //Replaced with continue
        continue;
      } 
      else if((*pte & PTE_P) != 0) {
        pa = PTE_ADDR(*pte);
        if(pa == 0)
          panic("kfree");
        char *v = P2V(pa);
        kfree(v);
        *pte = 0;
      }
    }
  } 
  //Mode 2: File Backed, write back to disk
  else if (flags & MAP_SHARED) {
    struct file *f = curproc->ofile[fd];

    if (!f) {
      return -1; // Invalid file descriptor.
    }
    f->off = 0;
    length = PGROUNDUP(length);
    
    for (int i = 0; i < length; i += PGSIZE) {
      // Read the file contents into the allocated page.
      //For some reason, addr + i works, even though I used mem for fileread? CHECK LATER
      int nread = filewrite(f, addr + i, PGSIZE);
      if (nread < 0) {
        return -1;
      }
    }
  }
  //Might not need to clear fields these out, since we are using a valid bit, but keeping it here for now. 
  curproc->mmaps[mmaps_index].start = (void *) -1;
  curproc->mmaps[mmaps_index].length = -1;
  curproc->mmaps[mmaps_index].flags = -1;
  curproc->mmaps[mmaps_index].fd = -1;
  curproc->mmaps[mmaps_index].valid = 0;
  curproc->mmaps[mmaps_index].has_guard = 0;
  curproc->mmaps[mmaps_index].ref--;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int sys_dup(void)
{
  struct file *f;
  int fd;

  if (argfd(0, 0, &f) < 0)
    return -1;
  if ((fd = fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if (argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return filewrite(f, p, n);
}

int sys_close(void)
{
  int fd;
  struct file *f;

  if (argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if (argfd(0, 0, &f) < 0 || argptr(1, (void *)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if (argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if ((ip = namei(old)) == 0)
  {
    end_op();
    return -1;
  }

  ilock(ip);
  if (ip->type == T_DIR)
  {
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if ((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if (dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0)
  {
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de))
  {
    if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if (de.inum != 0)
      return 0;
  }
  return 1;
}

// PAGEBREAK!
int sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if (argstr(0, &path) < 0)
    return -1;

  begin_op();
  if ((dp = nameiparent(path, name)) == 0)
  {
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if (namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if ((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if (ip->nlink < 1)
    panic("unlink: nlink < 1");
  if (ip->type == T_DIR && !isdirempty(ip))
  {
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if (writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if (ip->type == T_DIR)
  {
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

static struct inode *
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if ((dp = nameiparent(path, name)) == 0)
    return 0;
  ilock(dp);

  if ((ip = dirlookup(dp, name, 0)) != 0)
  {
    iunlockput(dp);
    ilock(ip);
    if (type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if ((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if (type == T_DIR)
  {              // Create . and .. entries.
    dp->nlink++; // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if (dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip;
}

int sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if (argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();

  if (omode & O_CREATE)
  {
    ip = create(path, T_FILE, 0, 0);
    if (ip == 0)
    {
      end_op();
      return -1;
    }
  }
  else
  {
    if ((ip = namei(path)) == 0)
    {
      end_op();
      return -1;
    }
    ilock(ip);
    if (ip->type == T_DIR && omode != O_RDONLY)
    {
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if ((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0)
  {
    if (f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  return fd;
}

int sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if (argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0)
  {
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;

  begin_op();
  if ((argstr(0, &path)) < 0 ||
      argint(1, &major) < 0 ||
      argint(2, &minor) < 0 ||
      (ip = create(path, T_DEV, major, minor)) == 0)
  {
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int sys_chdir(void)
{
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();

  begin_op();
  if (argstr(0, &path) < 0 || (ip = namei(path)) == 0)
  {
    end_op();
    return -1;
  }
  ilock(ip);
  if (ip->type != T_DIR)
  {
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  return 0;
}

int sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if (argstr(0, &path) < 0 || argint(1, (int *)&uargv) < 0)
  {
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for (i = 0;; i++)
  {
    if (i >= NELEM(argv))
      return -1;
    if (fetchint(uargv + 4 * i, (int *)&uarg) < 0)
      return -1;
    if (uarg == 0)
    {
      argv[i] = 0;
      break;
    }
    if (fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(path, argv);
}

int sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if (argptr(0, (void *)&fd, 2 * sizeof(fd[0])) < 0)
    return -1;
  if (pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0)
  {
    if (fd0 >= 0)
      myproc()->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}
