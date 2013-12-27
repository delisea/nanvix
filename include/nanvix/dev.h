/*
 * Copyright(C) 2011-2013 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * <nanvix/dev.h> - Uniform device interface library.
 */

#ifndef DEV_H_
#define DEV_H_

	#include <nanvix/const.h>
	#include <nanvix/fs.h>
	#include <sys/types.h>

	/* Device types. */
	#define CHRDEV 0 /* Charecter device. */
	#define BLKDEV 1 /* Block device.     */

	/*
	 * DESCRIPTION:
	 *   The MAJOR() macro returns the major number of a device.
	 */
	#define MAJOR(dev) \
		(((dev) >> 8) & 0xf)
	
	/*
	 * DESCRIPTION:
	 *   The MINOR() macro returns the minor number of a device.
	 */
	#define MINOR(dev) \
		(((dev) >> 4) & 0xf)
	
	/*
	 * DESCRIPTION:
	 *   The DEVID() macro returns the device number of a device.
	 */
	#define DEVID(major, minor, type) \
		(((major) << 8) | ((minor) << 4) | (type))
		
	/*
	 * DESCRIPTION:
	 *   The dev_init() function initializes the device drivers.
	 * 
	 * RETURN VALUE:
	 *   The dev_init() has no return value.
	 * 
	 * ERRORS:
	 *   No errors are defined.
	 */
	EXTERN void dev_init(void);

	/*========================================================================*
	 *                             character device                           *
	 *========================================================================*/
	
	/* Character device major numbers. */
	#define NULL_MAJOR 0x0 /* Null device. */
	#define TTY_MAJOR  0x1 /* tty device.  */
	
	/* NULL device. */
	#define NULL_DEV DEVID(NULL_MAJOR, 0, CHRDEV)
	
	/*
	 * Character device.
	 */
	struct cdev
	{
		int (*open)(unsigned);                            /* Open.  */
		ssize_t (*read)(unsigned, char *, size_t);        /* Read.  */
		ssize_t (*write)(unsigned, const char *, size_t); /* Write. */
	};
	
	/*
	 * DESCRIPTION:
	 *   The cdev_register() function attempts to register a character device
	 *   with major number major.
	 * 
	 * RETURN VALUE:
	 *   Upon successfull completion, the cdev_register() function returns 0. 
	 *   Upon failure, a negative error code is returned.
	 * 
	 * ERRORS:
	 *   - EINVAL: invalid major number.
	 *   - EBUSY: there is a device registered with the same major number.
	 */
	EXTERN int cdev_register(unsigned major, const struct cdev *dev);
	
	/*
	 * DESCRIPTION:
	 *   The cdev_write() function attempts to write n bytes from the buffer 
	 *   pointed to by buf to the character device identified by dev.
	 * 
	 * RETURN VALUE:
	 *   Upon successful completion, the cdev_write() function returns the 
	 *   number of bytes actually written to the device. Upon failure, a 
	 *   negative error code is returned.
	 * 
	 * ERRORS:
	 *   - EINVAL: invalid character device.
	 *   - ENOTSUP: operation not supported.
	 */
	EXTERN ssize_t cdev_write(dev_t dev, const void *buf, size_t n);
	
	/*
	 * DESCRIPTION:
	 *   The cdev_read() function attempts to read n bytes from the character 
	 *   device identified by dev to the buffer pointed to by buf.
	 * 
	 * RETURN VALUE:
	 *   Upon successfull completion, the cdev_read() function returns the 
	 *   number of bytes actually read from the device. Upon failure, a 
	 *   negative error code is returned.
	 * 
	 * ERRORS:
	 *   - EINVAL: invalid character device.
	 *   - ENOTSUP: operation not supported.
	 */
	EXTERN ssize_t cdev_read(dev_t dev, void *buf, size_t n);

	/*
	 * Opens a character device.
	 */
	EXTERN int cdev_open(dev_t dev);
	
	/*========================================================================*
	 *                               block device                             *
	 *========================================================================*/
	
	/* Block device major numbers. */
	#define RAMDISK_MAJOR 0x0 /* ramdisk device. */
	
	/*
	 * Block device.
	 */
	struct bdev
	{
		ssize_t (*read)(dev_t, char *, size_t, off_t);        /* Read.        */
		ssize_t (*write)(dev_t, const char *, size_t, off_t); /* Write.       */
		int (*readblk)(unsigned, struct buffer *);            /* Read block.  */
		int (*writeblk)(unsigned, struct buffer *);           /* Write block. */
	};
	
	/*
	 * DESCRIPTION:
	 *   The bdev_register() function attempts to register a block device
	 *   with major number major.
	 * 
	 * RETURN VALUE:
	 *   Upon successful completion, the bdev_register() function returns 0. 
	 *   Upon failure, a negative error code is returned.
	 * 
	 * ERRORS:
	 *   - EINVAL: invalid major number.
	 *   - EBUSY: there is a device registered with the same major number.
	 */
	EXTERN int bdev_register(unsigned major, const struct bdev *dev);

	/*
	 * DESCRIPTION:
	 *   The bdev_write() function attempts to write n bytes from the buffer 
	 *   pointed to by buf to the block device identified by dev, starting at
	 *   offset off.
	 * 
	 * RETURN VALUE:
	 *   Upon successful completion, the bdev_write() function returns the 
	 *   number of bytes actually written to the device. Upon failure, a 
	 *   negative error code is returned.
	 * 
	 * ERRORS:
	 *   - EINVAL: invalid block device.
	 *   - ENOTSUP: operation not supported.
	 */
	EXTERN ssize_t bdev_write(dev_t dev, const char *buf, size_t n, off_t off);

	/*
	 * DESCRIPTION:
	 *   The bdev_read() function attempts to read n bytes from the block 
	 *   device identified by dev to the buffer pointed to by buf, starting at
	 *   offset off.
	 * 
	 * RETURN VALUE:
	 *   Upon successfull completion, the bdev_read() function returns the 
	 *   number of bytes actually read from the device. Upon failure, a 
	 *   negative error code is returned.
	 * 
	 * ERRORS:
	 *   - EINVAL: invalid block device.
	 *   - ENOTSUP: operation not supported.
	 */
	EXTERN ssize_t bdev_read(dev_t dev, char *buf, size_t n, off_t off);
	
	/*
	 * Writes a block to a block device.
	 */
	EXTERN void bdev_writeblk(struct buffer *buf);
	
	/*
	 * Reads a block from a block device.
	 */
	EXTERN void bdev_readblk(struct buffer *buf);
	
#endif /* DEV_H_ */