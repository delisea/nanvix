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

/**
 * @brief Log 2 of block size.
 */
#define BLOCK_SIZE_LOG2 10

/**
 * @brief Block size (in bytes).
 */
 #define BLOCK_SIZE (1 << BLOCK_SIZE_LOG2)
	 
static void sleep(int ds) {
	struct tms timing; /* Timing information. */
	clock_t t;
	t = times(&timing);
	while(times(&timing)-t<100*ds);
}
static int testA()
{
	int fd;            /* File descriptor.    */
	char *buffer;      /* Buffer.             */
	fd = open("/sbin/fibi",  O_RDWR);
	/* Allocate buffer. */
	buffer = malloc(MEMORY_SIZE);
	if (buffer == NULL)
		exit(EXIT_FAILURE);
	
	// test éventuellement un peu long car bloc par bloc
	int cont;
	fd = open("/sbin/fibi",  O_RDWR);
	do {
		// Le prefecth du bloc suivant est automatique
		cont = read(fd, buffer, BLOCK_SIZE);
		sleep(1);// Simule un calcul
	} while(cont);
	close(fd);
	free(buffer);
	return (0);
}
static int testB()
{
	int fd;            /* File descriptor.    */
	char *buffer;      /* Buffer.             */
	fd = open("/sbin/fibi",  O_RDWR);
	/* Allocate buffer. */
	buffer = malloc(MEMORY_SIZE);
	if (buffer == NULL)
		exit(EXIT_FAILURE);
	
	int cont;
	fd = open("/sbin/fibi",  O_RDWR);
	do {
		// On prefetch spécifiquement l'espace mémoire utile suivant
		read(fd, 0, BLOCK_SIZE*20);
		sleep(4);// Simule un calcul
		cont = read(fd, buffer, BLOCK_SIZE*20);
	} while(cont);
	close(fd);
	free(buffer);
	return (0);
}

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
	printf("  testA   Version 1\n");
	printf("  testB   Version 2\n");
	
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
		if (!strcmp(argv[i], "testA"))
		{
			testA();
		}
		
		else if (!strcmp(argv[i], "testB"))
		{
			testB();
		}
		
		/* Wrong usage. */
		else
			usage();
	}
	
	return (EXIT_SUCCESS);
}
