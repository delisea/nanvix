/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2016-2016 Davidson Francis <davidsondfgl@gmail.com>
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

#include <assert.h>
#include <nanvix/config.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>

/* Test flags. */
#define EXTENDED (1 << 0)
#define FULL     (1 << 1)
#define VERBOSE  (1 << 2)

/* Test flags. */
static unsigned flags = VERBOSE;// | FULL

/*============================================================================*
 *                               swap_test                                    *
 *============================================================================*/

/**
 * @brief Swapping test module.
 * 
 * @details Forces swapping algorithms to be activated by performing a large
 *          matrix multiplication operation that does not fit on memory.
 * 
 * @returns Zero if passed on test, and non-zero otherwise.
 */
static int swap_test(void)
{
	#define N 1280
	int *a, *b, *c;
	clock_t t0, t1;
	struct tms timing;

	/* Allocate matrices. */
	if ((a = malloc(N*N*sizeof(int))) == NULL)
		goto error0;
	if ((b = malloc(N*N*sizeof(int))) == NULL)
		goto error1;
	if ((c = malloc(N*N*sizeof(int))) == NULL)
		goto error2;
		
	t0 = times(&timing);
	
	/* Initialize matrices. */
	for (int i = 0; i < N*N; i++)
	{
		a[i] = 1;
		b[i] = 1;
		c[i] = 0;
	}
	
	/* Multiply matrices. */
	if (flags & (EXTENDED | FULL))
	{	
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
					
				for (int k = 0; k < N; k++)
					c[i*N + j] += a[i*N + k]*b[k*N + j];
			}
		}
	}
	
	/* Check values. */
	if (flags & FULL)
	{
		for (int i = 0; i < N*N; i++)
		{
			if (c[i] != N)
				goto error3;
		}
	}
	
	/* House keeping. */
	free(a);
	free(b);
	free(c);
	
	t1 = times(&timing);
	
	/* Print timing statistics. */
	if (flags & VERBOSE)
		printf("  Elapsed: %d\n", t1 - t0);
	
	return (0);

error3:
	free(c);
error2:
	free(b);
error1:
	free(a);
error0:
	return (-1);
}

/*============================================================================*
 *                                  io_test                                   *
 *============================================================================*/

/**
 * @brief I/O testing module.
 * 
 * @details Reads sequentially the contents of the hard disk
            to a in-memory buffer.
 * 
 * @returns Zero if passed on test, and non-zero otherwise.
 */
static int io_test_bqp(void)
{
	int fd;            /* File descriptor.    */
	struct tms timing; /* Timing information. */
	clock_t t0, t1;    /* Elapsed times.      */
	char *buffer;      /* Buffer.             */
	
	//#define MEMORY_SIZE   0x1000000
	#define size_chunk   0x100
	#define nb_full_chunk   0x10000
	#define padding_chunk   0x0
	
	int parity = 0;
	
	/* Allocate buffer. */
	buffer = malloc(MEMORY_SIZE);//MEMORY_SIZE
	if (buffer == NULL)
		exit(EXIT_FAILURE);printf("1\n");
	
	/* Open hdd. */
	fd = open("/dev/hdd", O_RDONLY);
	//fd = open("/testotxt", O_RDONLY);
	if (fd < 0)
		exit(EXIT_FAILURE);printf("1\n");
		printf("p%d,%d\n", read(fd, buffer, MEMORY_SIZE), -1);
		printf("p%s,%d\n", strerror(errno), -1);
		printf("p%d,%d\n", read(fd, buffer, size_chunk), -1);
		printf("p%s,%d\n", strerror(errno), -1);
	
	t0 = times(&timing);
	
	for(int i = 0; i<nb_full_chunk; i++) {
		if (read(fd, buffer, size_chunk) != size_chunk)
		exit(EXIT_FAILURE);
		// Calculation of current and previous chunk parity
		for(int j = 0; j<size_chunk; j++) {
			parity = (parity + buffer[j])%2;
		}
	}
	
	// Add parity of padding chunk
	if (read(fd, buffer, padding_chunk) != padding_chunk)
		exit(EXIT_FAILURE);printf("1\n");
	for(int j = 0; j<padding_chunk; j++) {
		parity = (parity + buffer[j])%2;
	}
	
	/* Read hdd. */
	
	
	t1 = times(&timing);
	
	/* House keeping. */
	free(buffer);
	close(fd);
	
	/* Print timing statistics. */
	if (flags & VERBOSE)
		printf("parity is %i and time elapsed is %d\n", parity, t1 - t0);
	
	return (0);
}
static int io_test(void)
{
	int fd;            /* File descriptor.    */
	struct tms timing; /* Timing information. */
	clock_t t0, t1;    /* Elapsed times.      */
	char *buffer;      /* Buffer.             */
	
	/* Allocate buffer. */
	buffer = malloc(MEMORY_SIZE);
	if (buffer == NULL)
		exit(EXIT_FAILURE);
	
	/* Open hdd. */
	fd = open("/dev/hdd", O_RDONLY);
	//fd = open("/testotxt", O_RDONLY);
	if (fd < 0)
		exit(EXIT_FAILURE);
	
	t0 = times(&timing);
	
	/* Read hdd. */
	if (read(fd, buffer, MEMORY_SIZE) != MEMORY_SIZE)
		exit(EXIT_FAILURE);
	
	t1 = times(&timing);
	
	/* House keeping. */
	free(buffer);
	close(fd);
	
	/* Print timing statistics. */
	if (flags & VERBOSE)
		printf("  Elapsed: %d\n", t1 - t0);
	if(0)io_test_bqp();
	return (0);
}

/*static int io_test2(void)
{
	int fd;            * File descriptor.    *
	//system("touch testotxt\n");
	* Open hdd. **
	fd = open("/testotxt", O_WRONLY | O_CREAT);
	if (fd < 0)
		exit(EXIT_FAILURE);
	
	* Read hdd. **
	int i;
	for(i=0; i<1; i++)
	if (write(fd, "coucou", 5) != 5) {
		printf("fail\n");
		exit(EXIT_FAILURE);
	}
	
	close(fd);
	if(0) io_test_bqp();
	return (0);
}*/
/**
	 * @brief Log 2 of block size.
	 */
	#define BLOCK_SIZE_LOG2 10
	
	/**
	 * @brief Block size (in bytes).
	 */
	#define BLOCK_SIZE (1 << BLOCK_SIZE_LOG2)
#define TAILLE_FICHIER 500000
int Buff[TAILLE_FICHIER];
static void sleep(int ds) {
	struct tms timing; /* Timing information. */
	clock_t t;
	t = times(&timing);//static int kkk = 0;
	while(times(&timing)-t<100*ds);//printf("%d,%d\n", times(&timing)-t, kkk);kkk++;
}
static int io_test2(void)
{
	int fd;            /* File descriptor.    */
	struct tms timing; /* Timing information. */
	clock_t t0, t1;/*, ta, tb;*/    /* Elapsed times.      */
	char *buffer;      /* Buffer.             */
	//fd = open("/sbin/fibi",  O_RDWR);
	/* Allocate buffer. */
	buffer = malloc(MEMORY_SIZE);
	if (buffer == NULL)
		exit(EXIT_FAILURE);
	
	//while(1){printf("%d\n", times(&timing));}
	//while(1){sleep(5);printf("bip\n");}
	/*t0 = times(&timing);
		
	printf("%d\n",read(fd, buffer, BLOCK_SIZE*80));
	
	t1 = times(&timing);
	
	printf("Elapsed: %d\n", t1 - t0);
	
	read(fd, 0, BLOCK_SIZE*80);
	
	
	t2 = times(&timing);
	while(times(&timing)-t2<1000);
	//while(times(&timing)<10000);
	
	t0 = times(&timing);
		
	printf("%d\n",read(fd, buffer, BLOCK_SIZE*80));
	
	t1 = times(&timing);
	
	printf("Elapsed: %d\n", t1 - t0);
	
	
	close(fd);
	fd = open("/sbin/fibii",  O_RDWR);
	
	
	
	
	t2 = times(&timing);
	while(times(&timing)-t2<1000);
	//while(times(&timing)<10000);
	
	t0 = times(&timing);
		
	printf("%d\n",read(fd, buffer, BLOCK_SIZE*80));
	
	t1 = times(&timing);
	
	printf("Elapsed: %d\n", t1 - t0);
	
	
	t0 = times(&timing);
		
	printf("%d\n",read(fd, buffer, BLOCK_SIZE*80));
	
	t1 = times(&timing);
	
	printf("Elapsed: %d\n", t1 - t0);
	
	read(fd, 0, BLOCK_SIZE*80);
	close(fd);*/
	
	
	
	
	int cont;
	
	
	
	/*fd = open("/sbin/fibii",  O_RDWR);
	t0 = times(&timing);
	do {
	sleep(4);
		cont = read(fd, buffer, BLOCK_SIZE*20);
	//printf("%d\n",cont = read(fd, buffer, BLOCK_SIZE*80));
	
	} while(cont);
	t1 = times(&timing);
	
	printf("Elapsed: %d\n", t1 - t0);
	close(fd);*/
	fd = open("/sbin/fibiii",  O_RDWR);
	
	t0 = times(&timing);
	do {
		read(fd, 0, BLOCK_SIZE*20);
		sleep(4);
	//while(times(&timing)<10000);
	
	
		cont += 100000;//cont = read(fd, buffer, BLOCK_SIZE*20);
	//printf("%d\n",cont = read(fd, buffer, BLOCK_SIZE*80));
	
	} while(cont);
	t1 = times(&timing);
	
	printf("Elapsed: %d\n", t1 - t0);
	
	close(fd);
	
	
	
	
	
	
		
	/*int parity = 0;
	fd = open("/sbin/fibi",  O_RDWR);
	read(fd, 0, BLOCK_SIZE*90);
	t0 = times(&timing);
	for(unsigned int i=0;i<(TAILLE_FICHIER* sizeof(int))/BLOCK_SIZE;i++){
		read(fd, buffer, BLOCK_SIZE);
		for(int j = 0; j<BLOCK_SIZE; j++) {
			parity = (parity + buffer[j])%2;
			//start_t2 = times(&timing2);
			//while(times(&timing2)-start_t2<10)printf("p%d,%d",times(&timing2),start_t2);
		}
	}*/
	//read(fd, 0, BLOCK_SIZE*26);
	//for(unsigned int i=0;read(fd, buffer, BLOCK_SIZE)==BLOCK_SIZE;i++){
		/*if(i%5==4)
			read(fd, 0, BLOCK_SIZE*5);*/
	//	printf("%d,",i);
		/*for(int j = 0; j<BLOCK_SIZE; j++) {
			parity = (parity + buffer[j]*buffer[j]+buffer[j]*parity)%2;
			for(int kkk = 0; kkk<100; kkk++);
		}*/
		/*ta = times(&timing);
		tb = times(&timing);
		while(tb-ta<2) {
			tb = times(&timing);
			//printf("p%d,%d",ta,tb);
		}*/
	//}
	
	/*if (read(fd, buffer, BLOCK_SIZE) != BLOCK_SIZE)
		exit(EXIT_FAILURE);*/
	printf("Fin lecture\n");
	t1 = times(&timing);
	
	/* House keeping. */
	free(buffer);
	
	/* Print timing statistics. */
	//if (flags & VERBOSE)
		//printf("Parity: %d\n  Elapsed: %d\n", parity, t1 - t0);
	
	return (0);
}

static void fichi() {
	int fd;            /* File descriptor.    */
	char *buffer;      /* Buffer.             */
	
	/* Allocate buffer. */
	buffer = malloc(MEMORY_SIZE);
	if (buffer == NULL)
		exit(EXIT_FAILURE);
	
	printf("Create the file\n");
	/*Create the file*/
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	char *filename = "/testStudent";
	fd = creat(filename, mode);
	close(fd);
	
	printf("Open the file\n");
	/* Open the file */
	fd = open("/testStudent",  O_RDWR);
	if (fd < 0)
		exit(EXIT_FAILURE);
	
	unsigned int i;
	for(i=0;i<TAILLE_FICHIER;i++){
		Buff[i]=i;
	}
	write(fd,Buff,TAILLE_FICHIER);
	
	printf("Read the file\n");
	/* Read hdd. */
	
	close(fd);
}


/*============================================================================*
 *                                sched_test                                  *
 *============================================================================*/

/**
 * @brief Performs some dummy CPU-intensive computation.
 */
static void work_cpu(void)
{
	int c;
	
	c = 0;
		
	/* Perform some computation. */
	for (int i = 0; i < 4096; i++)
	{
		int a = 1 + i;
		for (int b = 2; b < i; b++)
		{
			if ((i%b) == 0)
				a += b;
		}
		c += a;
	}
}

/**
 * @brief Performs some dummy IO-intensive computation.
 */
static void work_io(void)
{
	int fd;            /* File descriptor. */
	char buffer[2048]; /* Buffer.          */
	
	/* Open hdd. */
	fd = open("/dev/hdd", O_RDONLY);
	if (fd < 0)
		_exit(EXIT_FAILURE);
	
	/* Read data. */
	for (size_t i = 0; i < MEMORY_SIZE; i += sizeof(buffer))
	{
		if (read(fd, buffer, sizeof(buffer)) < 0)
			_exit(EXIT_FAILURE);
	}
	
	/* House keeping. */
	close(fd);
}

/**
 * @brief Scheduling test 0.
 * 
 * @details Spawns two processes and tests wait() system call.
 * 
 * @return Zero if passed on test, and non-zero otherwise.
 */
static int sched_test0(void)
{
	pid_t pid;
	
	pid = fork();
	
	/* Failed to fork(). */
	if (pid < 0)
		return (-1);
	
	/* Child process. */
	else if (pid == 0)
	{
		work_cpu();
		_exit(EXIT_SUCCESS);
	}
	
	wait(NULL);
	
	return (0);
}

/**
 * @brief Scheduling test 1.
 * 
 * @details Forces the priority inversion problem, to check how well the system
 *          performs on dynamic priorities.
 * 
 * @returns Zero if passed on test, and non-zero otherwise.
 */
static int sched_test1(void)
{
	pid_t pid;
		
	pid = fork();
	
	/* Failed to fork(). */
	if (pid < 0)
		return (-1);
	
	/* Parent process. */
	else if (pid > 0)
	{
		nice(-2*NZERO);
		work_cpu();
	}
	
	/* Child process. */
	else
	{
		nice(2*NZERO);
		work_io();
		_exit(EXIT_SUCCESS);
	}
		
	wait(NULL);
	
	return (0);
}

/**
 * @brief Scheduling test 1.
 * 
 * @details Spawns several processes and stresses the scheduler.
 * 
 * @returns Zero if passed on test, and non-zero otherwise.
 */
static int sched_test2(void)
{
	pid_t pid[4];
	
	for (int i = 0; i < 4; i++)
	{
		pid[i] = fork();
	
		/* Failed to fork(). */
		if (pid[i] < 0)
			return (-1);
		
		/* Child process. */
		else if (pid[i] == 0)
		{
			if (i & 1)
			{
				nice(2*NZERO);
				work_cpu();
				_exit(EXIT_SUCCESS);
			}
			
			else
			{	
				nice(-2*NZERO);
				pause();
				_exit(EXIT_SUCCESS);
			}
		}
	}
	
	for (int i = 0; i < 4; i++)
	{
		if (i & 1)
			wait(NULL);
			
		else
		{	
			kill(pid[i], SIGCONT);
			wait(NULL);
		}
	}
	
	return (0);
}

/*============================================================================*
 *                             Semaphores Test                                *
 *============================================================================*/

/**
 * @brief Creates a semaphore.
 */
#define SEM_CREATE(a, b) (assert(((a) = semget(b)) >= 0))

/**
 * @brief Initializes a semaphore.
 */
#define SEM_INIT(a, b) (assert(semctl((a), SETVAL, (b)) == 0))

/**
 * @brief Destroys a semaphore.
 */
#define SEM_DESTROY(x) (assert(semctl((x), IPC_RMID, 0) == 0))

/**
 * @brief Ups a semaphore.
 */
#define SEM_UP(x) (assert(semop((x), 1) == 0))

/**
 * @brief Downs a semaphore.
 */
#define SEM_DOWN(x) (assert(semop((x), -1) == 0))

/**
 * @brief Puts an item in a buffer.
 */
#define PUT_ITEM(a, b)                                \
{                                                     \
	assert(lseek((a), 0, SEEK_SET) != -1);            \
	assert(write((a), &(b), sizeof(b)) == sizeof(b)); \
}                                                     \

/**
 * @brief Gets an item from a buffer.
 */
#define GET_ITEM(a, b)                               \
{                                                    \
	assert(lseek((a), 0, SEEK_SET) != -1);           \
	assert(read((a), &(b), sizeof(b)) == sizeof(b)); \
}                                                    \

/**
 * @brief Producer-Consumer problem with semaphores.
 * 
 * @details Reproduces consumer-producer scenario using semaphores.
 * 
 * @returns Zero if passed on test, and non-zero otherwise.
 */
int semaphore_test3(void)
{
	pid_t pid;                  /* Process ID.              */
	int buffer_fd;              /* Buffer file descriptor.  */
	int empty;                  /* Empty positions.         */
	int full;                   /* Full positions.          */
	int mutex;                  /* Mutex.                   */
	const int BUFFER_SIZE = 32; /* Buffer size.             */
	const int NR_ITEMS = 512;   /* Number of items to send. */
	
	/* Create buffer.*/
	buffer_fd = open("buffer", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (buffer_fd < 0)
		return (-1);
	
	/* Create semaphores. */
	SEM_CREATE(mutex, 1);
	SEM_CREATE(empty, 2);
	SEM_CREATE(full, 3);
		
	/* Initialize semaphores. */
	SEM_INIT(full, 0);
	SEM_INIT(empty, BUFFER_SIZE);
	SEM_INIT(mutex, 1);
	
	if ((pid = fork()) < 0)
		return (-1);
	
	/* Producer. */
	else if (pid == 0)
	{
		for (int item = 0; item < NR_ITEMS; item++)
		{
			SEM_DOWN(empty);
			SEM_DOWN(mutex);
			
			PUT_ITEM(buffer_fd, item);
				
			SEM_UP(mutex);
			SEM_UP(full);
		}

		_exit(EXIT_SUCCESS);
	}
	
	/* Consumer. */
	else
	{
		int item;
		
		do
		{
			SEM_DOWN(full);
			SEM_DOWN(mutex);
			
			GET_ITEM(buffer_fd, item);
				
			SEM_UP(mutex);
			SEM_UP(empty);
		} while (item != (NR_ITEMS - 1));
	}
					
	/* Destroy semaphores. */
	SEM_DESTROY(mutex);
	SEM_DESTROY(empty);
	SEM_DESTROY(full);
	
	close(buffer_fd);
	unlink("buffer");
	
	return (0);
}

/*============================================================================*
 *                                FPU test                                    *
 *============================================================================*/

/**
 * @brief Performs some dummy FPU-intensive computation.
 */
static void work_fpu(void)
{
	const int n = 16; /* Matrix size.    */
	float a[16][16];  /* First operand.  */
	float b[16][16];  /* Second operand. */
	float c[16][16];  /* Result.         */
	
	/* Initialize matrices. */
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			a[i][j] = 1.0;
			a[i][j] = 2.0;
			c[i][j] = 0.0;
		}
	}
	
	/* Perform matrix multiplication. */
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			for (int k = 0; k < n; k++)
				c[i][j] += a[i][k]*b[k][i];
		}
	}
}

/**
 * @brief FPU testing module.
 * 
 * @details Performs a floating point operation, while trying to
            mess up the stack from another process.
 * 
 * @returns Zero if passed on test, and non-zero otherwise.
 */
int fpu_test(void)
{
	pid_t pid;     /* Child process ID.     */
	float a = 6.7; /* First dummy operand.  */
	float b = 1.2; /* Second dummy operand. */
	float result;  /* Result.               */
	
	/* Perform a/b */
	__asm__ volatile(
		"flds %1;"
		"flds %0;"
		"fdivrp %%st,%%st(1);"
		: /* noop. */
		: "m" (b), "m" (a)
	);

	pid = fork();
	
	/* Failed to fork(). */
	if (pid < 0)
		return (-1);
	
	/*
	 * Child process tries
	 * to mess up the stack.
	 */
	else if (pid == 0)
	{
		work_fpu();
		_exit(EXIT_SUCCESS);
	}
	
	wait(NULL);

	/* But it's only in your context,
	 * so nothing changed for father process.
	 */
	__asm__ volatile(
		"fstps %0;"
		: "=m" (result)
	);

	/* 0x40b2aaaa = 6.7/1.2 = 5.5833.. */
	return (result == 0x40b2aaaa);
}

/*static void write_test_file(void)
{
	int fd;            
	char buffer[2048]; 
	
	fd = open("testfile", O_WRONLY | O_CREAT);
	if (fd < 0)
		_exit(EXIT_FAILURE);
	
	for (size_t i = 0; i < MEMORY_SIZE; i += sizeof(buffer))
	{
		if (write(fd, "ab", 3) < 0)
			_exit(EXIT_FAILURE);
	}
	
	close(fd);
}*/


/*============================================================================*
 *                                   main                                     *
 *============================================================================*/

/**
 * @brief Prints program usage and exits.
 * 
 * @details Prints the program usage and exists gracefully.
 */
static void usage(void)
{
	printf("Usage: test [options]\n\n");
	printf("Brief: Performs regression tests on Nanvix.\n\n");
	printf("Options:\n");
	printf("  fpu   Floating Point Unit Test\n");
	printf("  io    I/O Test\n");
	printf("  ipc   Interprocess Communication Test\n");
	printf("  swp   Swapping Test\n");
	printf("  sched Scheduling Test\n");
	
	exit(EXIT_SUCCESS);
}

/**
 * @brief System testing utility.
 */
int main(int argc, char **argv)
{
	/* Missing arguments? */
	if (argc <= 1)
		usage();

	for (int i = 1; i < argc; i++)
	{
		/* I/O test. */
		if (!strcmp(argv[i], "io"))
		{
			printf("I/O Test\n");
			printf("  Result:             [%s]\n", 
				(!io_test()) ? "PASSED" : "FAILED");
		}
		
		/* Swapping test. */
		else if (!strcmp(argv[i], "swp"))
		{
			printf("Swapping Test\n");
			printf("  Result:             [%s]\n",
				(!swap_test()) ? "PASSED" : "FAILED");
		}
		
		/* Scheduling test. */
		else if (!strcmp(argv[i], "sched"))
		{
			printf("Scheduling Tests\n");
			printf("  waiting for child  [%s]\n",
				(!sched_test0()) ? "PASSED" : "FAILED");
			printf("  dynamic priorities [%s]\n",
				(!sched_test1()) ? "PASSED" : "FAILED");
			printf("  scheduler stress   [%s]\n",
				(!sched_test2()) ? "PASSED" : "FAILED");
		}
		
		/* IPC test. */
		else if (!strcmp(argv[i], "ipc"))
		{
			printf("Interprocess Communication Tests\n");
			printf("  producer consumer [%s]\n",
				(!semaphore_test3()) ? "PASSED" : "FAILED");
		}

		/* FPU test. */
		else if (!strcmp(argv[i], "fpu"))
		{
			printf("Float Point Unit Test\n");
			printf("  Result [%s]\n",
				(!fpu_test()) ? "PASSED" : "FAILED");
		}

		/* Write test. */
		else if (!strcmp(argv[i], "wtf"))
		{
			printf("Write testfile\n");
			io_test2();//write_test_file();
		}

		/* Write test. */
		else if (!strcmp(argv[i], "fichi"))
		{
			printf("Write testfile\n");
			fichi();//write_test_file();
		}
	
	
		/* Wrong usage. */
		else
			usage();
	}
	
	return (EXIT_SUCCESS);
}
