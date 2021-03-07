#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <synch.h>
#include <addrspace.h>
#include <copyinout.h>
#include "opt-A2.h" 
#include <mips/trapframe.h>

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
  *retval = curproc->pid;
  return 0;
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}

#if OPT_A2

pid_t sys_fork(pid_t *retval, struct trapframe *tf) {
  
  // create new process structure for child process
  struct proc* childProc = proc_create_runprogram("child1"); // what do I name this??
  if (childProc == NULL) { //NULL case
    //panic("childProc is NULL");
    return 1;
  }

  // create and copy address space and data from parent to child
  spinlock_acquire(&childProc->p_lock);
  int errorCode = as_copy(curproc_getas(), &(childProc->p_addrspace));
  spinlock_release(&childProc->p_lock);
  if (errorCode != 0) {
    //panic("Invalid Address Space"); // as_copy errored out
    return 1;
  }

  // add new child to curproc's children array
  spinlock_acquire(&childProc->p_lock);
  array_add(curproc->children, childProc, NULL); // add current child to current process children array
  childProc->parent = curproc; // pointer to sole parent process
  spinlock_release(&childProc->p_lock);

  // create a thread for the child process
  spinlock_acquire(&childProc->p_lock);
  struct trapframe *child_tf = kmalloc(sizeof(struct trapframe)); //??
  memcpy(child_tf, tf, sizeof(struct trapframe)); //deep copy
  spinlock_release(&childProc->p_lock);
  if (child_tf == NULL) {
    //panic("Can't allocate space for child trapframe");
    return 1;
  }
  
  //spinlock_acquire(&childProc->p_lock);
  errorCode = thread_fork("childProcThread", childProc, (void*)enter_forked_process, (void*)child_tf, 0); // what's the number for
  if (errorCode != 0) {
    return 1; // failed thread_fork
  }
  

  *retval = childProc->pid; // return value? idk

  //spinlock_release(&childProc->p_lock); 

  return 0; // success

}

#endif /* OPT_A2 */
