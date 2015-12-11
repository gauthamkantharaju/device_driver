#ifndef _MAIN_H_
#define _MAIN_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/moduleparam.h>
#include <linux/atomic.h>

#define CONFIG_UACCESS	1

#if (CONFIG_UACCESS == 1)
#  include <linux/uaccess.h>
#else
#  error "Cannot include uaccess"
#endif

#define SUCCESS				0
#define ERROR				-1
#define MAX_STRLEN			50
#define MAX_LEN				5

#define MODULE_NAME			"hello"
#define MOD_VERSION			"1.0"

#define IOCTL_MAX_CMDS		2
#define IOCTL_HELLO_MAGIC	'h'
#define IOCTL_HELLO_TEST1	_IO(IOCTL_HELLO_MAGIC, 0)
#define IOCTL_HELLO_TEST2	_IO(IOCTL_HELLO_MAGIC, 1)

#if 1
static inline void iline(char *str)
{
	pr_info("String:%s\n", str);
}
#endif

typedef struct _context {
	char *buf;
	int *arry;
	unsigned int len;
	unsigned int rindex;
	unsigned int windex;
	spinlock_t lock;
} CNTXT;

#endif /* _MAIN_H_ */
