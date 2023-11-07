#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  ////////////////////////////////
  //Lazy Alloc? Might do it? and MAP_GROWSUP
  case T_PGFLT:
    int fault_addr = rcr2();
    if(fault_addr % PGSIZE != 0) {
      fault_addr = PGROUNDDOWN(fault_addr);
    }
    //Flag to see if a correct grows up fault happened. If still 0 at the end, then kill the program
    int grows_up = 0;
    struct proc *curproc = myproc();
    for (int i = 0; i < MAX_MMAPS; i++) {
      char *guard_page_virt = (char *)(curproc->mmaps[i].start) + curproc->mmaps[i].length;
      if(curproc->mmaps[i].valid && curproc->mmaps[i].has_guard && guard_page_virt == (char *) PGROUNDDOWN(fault_addr)) {
        //Print guard page virt addr and fault addr
        char *mem = kalloc();
        if (mappages(curproc->pgdir, guard_page_virt, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
            curproc->killed = 1;
            grows_up = 0;
        }
        grows_up = 1;
        curproc->mmaps[i].length += PGSIZE;

        //Returns non-zero if there is already a mapping here. This means we should kill the program, since we don't have enough space for the new guard page
        pte_t *pte;
        pte = walkpgdir(curproc->pgdir, (void *) guard_page_virt + PGSIZE, 0);
        if(*pte !=  0) {
          curproc->killed = 1;
          grows_up = 0;
          break;
        } 
        else {
          //Set the guard page
          walkpgdir(curproc->pgdir, guard_page_virt + PGSIZE, 1);
          break;
        }
      }
    }
    //If not accessing guard page (or doing lazy alloc, if we implement it), then kill the program
    if(!grows_up) {
      curproc->killed = 1;
      cprintf("Segmentation Fault\n");
    } else {
      break;
    }
  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
