/*
 * Copyright (C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * sys/fork.c - fork() system call implementation.
 */

#include <i386/i386.h>
#include <nanvix/const.h>
#include <nanvix/klib.h>
#include <nanvix/paging.h>
#include <nanvix/pm.h>
#include <sys/types.h>
#include <errno.h>

/*
 * Creates a new process.
 */
PUBLIC pid_t sys_fork(void)
{
	int i;                /* Loop index.     */
	int err;              /* Error?          */
	struct process *proc; /* Process.        */
	struct region *reg;   /* Memory region.  */
	struct pregion *preg; /* Process region. */

	/* Search for free process. */
	for (proc = FIRST_PROC; proc <= LAST_PROC; proc++)
	{
		/* Found. */
		if (proc->flags & PROC_FREE)
			goto found;
	}

	kprintf("process table overflow");
	
	return (-EAGAIN);

found:
	
	/* Mark process as beeing created. */
	proc->flags = PROC_NEW & ~PROC_FREE;

	err = crtpgdir(proc);
	
	/* Failed to create process page directory. */
	if (err)
		goto error0;
		
	/*
	 * Duplicate attached regions.
	 * Notice that regions will be attached in the child process
	 * on the same indexes as in the father process.
	 */
	for (i = 0; i < NR_PREGIONS; i++)
	{
		preg = &curr_proc->pregs[i];
		
		/* Process region not in use. */
		if (preg->reg == NULL)
			continue;
		
		lockreg(preg->reg);
		reg = dupreg(preg->reg);
		unlockreg(preg->reg);
			
		/* Failed to duplicate region. */
		if (reg == NULL)
			goto error1;
			
		err = attachreg(proc, &proc->pregs[i], preg->start, reg);
			
		/* Failed to attach region. */
		if (err)
		{
			freereg(reg);
			goto error1;
		}
			
		unlockreg(reg);
	}
	
	/* Initialize process. */
	proc->intlvl = curr_proc->intlvl;
	proc->received = 0;
	for (i = 0; i < NR_SIGNALS; i++)
		proc->handlers[i] = curr_proc->handlers[i];
	proc->size = curr_proc->size;
	proc->pwd = curr_proc->pwd;
	proc->pwd->count++;
	proc->root = curr_proc->root;
	proc->root->count++;
	for (i = 0; i < OPEN_MAX; i++)
	{
		proc->ofiles[i] = curr_proc->ofiles[i];
		
		/* Increment file reference count. */
		if (proc->ofiles[i] != NULL)
			proc->ofiles[i]->count++;
	}
	proc->close = curr_proc->close;
	proc->umask = curr_proc->umask;
	proc->tty = curr_proc->tty;
	proc->status = 0;
	proc->nchildren = 0;
	proc->uid = curr_proc->uid;
	proc->euid = curr_proc->euid;
	proc->suid = curr_proc->suid;
	proc->gid = curr_proc->gid;
	proc->egid = curr_proc->egid;
	proc->sgid = curr_proc->sgid;
	proc->pid = next_pid++;
	proc->pgrp = curr_proc->pgrp;
	proc->father = curr_proc;
	proc->utime = 0;
	proc->ktime = 0;
	proc->priority = curr_proc->priority;
	proc->nice = curr_proc->nice;
	proc->alarm = 0;
	proc->next = NULL;
	proc->chain = NULL;
	sched(proc);
	
	curr_proc->nchildren++;
	
	return (proc->pid);

error1:
	/* Detach attached regions. */
	while (--i >= 0)
	{
		/* Region not attached. */
		if (proc->pregs[i].reg == NULL)
			continue;
		
		/* Detach. */
		preg = &proc->pregs[i];
		lockreg(preg->reg);
		detachreg(proc, preg);
	}
error0:
	dstrypgdir(proc);
	proc->flags = PROC_FREE;
	return (-ENOMEM);
}