#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

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
sys_getppid(void)
//Se crea un nuevo syscall para obtener el ID del proceso padre
{
  struct proc *curr_proc = myproc();  //Puntero al proceso actual
  if(curr_proc->parent == (void*)0){ //Este void*0 es NULL 
    //Si el proceso actual no tiene padre, se devuelve -1
    return -1;
  }
  return curr_proc->parent->pid; //Retorna el ID del proceso padre
}

uint64
sys_getancestror(void)
/* 
Se crea un nuevo syscall para obtener el ID de un ancestro en una generación determinada. Si la generación no existe, devuelve -1
Notar que Generación 0 es el proceso actual, generación 1 es el proceso padre, generación 2 es el proceso abuelo y así sucesivamente.
*/
{
  int num_gen; //Variable para obtener el número de generaciones desde el argumento de la función
  argint(0,&num_gen); //Obtiene el número de generaciones desde el argumento de la función
  int cont_gen = 0;                                           
  struct proc *curr_proc = myproc();// Puntero que recorre la lista de procesos ancestros
  while(curr_proc->parent != (void*)0 && cont_gen < num_gen){ //Lo recorre hasta llegar a la generación deseada o al proceso raíz
    cont_gen++; 
    curr_proc = curr_proc->parent; //Avanzar al proceso padre de cada proceso
  }
  if(cont_gen == num_gen){ //Si se llega a la generación deseada, devuelve el ID de ese proceso ancestro
    return curr_proc->pid; 
  }
  return -1; // Si no, devuelve -1 (se llegó al proceso raíz)
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
