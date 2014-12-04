/*
 * Copyright(C) 2011-2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/const.h>
#include <nanvix/fs.h>
#include <nanvix/hal.h>
#include <nanvix/klib.h>
#include <nanvix/dev.h>
#include <nanvix/pm.h>
#include <nanvix/mm.h>
#include <nanvix/syscall.h>
#include <fcntl.h>

/**
 * @brief Forks the current process.
 * 
 * @returns For the parent process, the process ID of the child process. For
 *          the child process zero is returned. Upon failure, a negative error
 *          code is returned instead.
 */
pid_t fork(void)
{
	pid_t pid;
	
	__asm__ volatile (
		"int $0x80"
		: "=a" (pid)
		: "0" (NR_fork)
	);
	
	/* Error. */
	if (pid < 0)
		return (-1);
	
	return (pid);
}

/**
 * @brief Executes a program.
 * 
 * @param filename Program to be executed.
 * @param argv     Arguments variables to pass to the program.
 * @param envp     Environment variables to pass to the program.
 * 
 * @returns Upon successful completion, this function shall not return. Upon
 *          failure, it does return with a negative error code.
 */
int execve(const char *filename, const char **argv, const char **envp)
{
	int ret;
	
	__asm__ volatile (
		"int $0x80"
		: "=a" (ret)
		: "0" (NR_execve),
		  "b" (filename),
		  "c" (argv),
		  "d" (envp)
	);
	
	/* Error. */
	if (ret)
		return (-1);
	
	return (ret);
}

/**
 * @brief Exits the current process.
 * 
 * @param status Exit status.
 */
void _exit(int status)
{
	__asm__ volatile(
		"int $0x80"
		: /* empty. */
		: "a" (NR__exit),
		"b" (status)
	);
}

/**
 * @brief opens a file.
 * 
 * @param path  File path.
 * @param oflag Open flags.
 * 
 * @returns Upon successful completion, zero is returned. Upon failure, a 
 *          negative error code is returned instead.
 */
int open(const char *path, int oflag, ...)
{
	int ret;
	
	__asm__ volatile (
		"int $0x80"
		: "=a" (ret)
		: "0" (NR_open),
		  "b" (path),
		  "c" (oflag),
		  "d" (0)
	);
	
	/* Error. */
	if (ret < 0)
		return (-1);
	
	return (ret);
}

/**
 * @brief Init process.
 */
PRIVATE void init(void)
{
	const char *argv[] = { "init", "/etc/inittab", NULL };
	const char *envp[] = { "PATH=/bin:/sbin", "HOME=/", NULL };
	
	/* Open standard output streams. */
	open("/dev/tty", O_RDONLY);
	open("/dev/tty", O_WRONLY);
	open("/dev/tty", O_WRONLY);
		
	execve("/sbin/init", argv, envp);
}

/**
 * @brief Initializes the kernel.
 */
PUBLIC void kmain(void)
{		
	pid_t pid;
	
	/* Initialize system modules. */
	dev_init();
	mm_init();
	pm_init();
	fs_init();
	
	/* Spawn init process. */
	if ((pid = fork()) < 0)
		kpanic("failed to fork idle process");
	else if (pid == 0)
	{	
		init();
		kprintf("failed to execute init");
		_exit(-1);
	}
	
	/* idle process. */	
	while (1)
	{
		halt();
		yield();
	}
}
