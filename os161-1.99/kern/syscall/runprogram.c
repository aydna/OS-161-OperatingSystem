/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include "opt-A2.h"
#include <copyinout.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
#if OPT_A2
int runprogram(char *progname, int numArgs, char** argsArray)
#else
int runprogram(char *progname)
#endif // OPT_A2
{
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* We should be a new process. */
	KASSERT(curproc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as ==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	curproc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

#if OPT_A2
	// argument passing stuff

	vaddr_t currStackPtr = USERSTACK; // top of stack initially
  	vaddr_t* argAddresses = kmalloc((numArgs + 1) * sizeof(vaddr_t)); // array of stack addresses stored for each arg
	
	for (int i = numArgs - 1; i >= 0; i--) {
		size_t currArgSize = (strlen(argsArray[i]) + 1) * sizeof(char); // chars are size 1; no need for alignment
		currArgSize = ROUNDUP(currArgSize, 4); // do we need this to enforce?
		currStackPtr -= currArgSize; // update the stack ptr
		argAddresses[i] = currStackPtr;
		copyoutstr((const char*)argsArray[i], (userptr_t)currStackPtr, currArgSize, NULL); // write curr arg string onto stack
  	}
  	// take care of null terminate
  	argAddresses[numArgs] = (vaddr_t)NULL;

	// take care of the pointers array now
  	//currStackPtr = ROUNDUP(currStackPtr, 4) - 4; // to ensure that it is currently 8 aligned for ptrs
  	const size_t ptrSize = sizeof(vaddr_t); // 4 bits
  	for (int i = numArgs; i >= 0 ; i--) {
    	currStackPtr -= ptrSize;
    	copyout((void*)&argAddresses[i], (userptr_t)currStackPtr, ptrSize); // write ptr addr of arg i onto stack
  	}
	
	//as_destroy(oldAddressSpace);
  
  	enter_new_process(numArgs, (userptr_t)currStackPtr, currStackPtr, entrypoint);

#else

	/* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  stackptr, entrypoint);

#endif // OPT_A2

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}

