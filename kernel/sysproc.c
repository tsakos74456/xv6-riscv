#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_getpinfo(void)
{

  struct pstat kernel_st;
  struct pstat *user_st;

  argaddr(0, (uint64*)&user_st);
  if (user_st == 0)
    return -1;

  struct proc *p;
  int i = 0;

  memset(&kernel_st, 0, sizeof(kernel_st));

  // parent pointer is protected by wait_lock so there won't be any data race and parent won't change 
  acquire(&wait_lock);
  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state != UNUSED) {
      kernel_st.in_use[i] = 1;
      kernel_st.pid[i] = p->pid;
      kernel_st.ppid[i] = p->parent ? p->parent->pid : -1;
      kernel_st.state[i] = p->state;
      kernel_st.priority[i] = p->priority;
      kernel_st.size[i] = p->sz;
      safestrcpy(kernel_st.name[i], p->name, sizeof(kernel_st.name[i]));
      i++;
    }
    release(&p->lock);
  }
  release(&wait_lock);

  kernel_st.num_processes = i;

  // copy to the user struct
  if (copyout(myproc()->pagetable, (uint64)user_st,
              (char *)&kernel_st, sizeof(kernel_st)) < 0)
    return -1;
  return 0;
}
