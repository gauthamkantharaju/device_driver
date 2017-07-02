/**
	Module version	: 1.0
	Author			: Gautham Kantharaju
	Description		: Simple device driver with debugfs
 */

#include "main.h"

static int majorno;
/* Non-Static variable */
int len = 5;
#if 0
	EXPORT_SYMBOL(len);
#endif
atomic_t flag = ATOMIC_INIT(0);

int x = 1;
module_param(x, bool, 0);
MODULE_PARM_DESC(x, "Boolean value, default is 1");

static char *str = "Simple Hello Device driver";
module_param(str, charp, 0);
MODULE_PARM_DESC(str,
		"Character pointer, default string : Simple Hello Device Driver");

static int arr[MAX_LEN];
static int ptr;
module_param_array(arr, int, &ptr, 0);
MODULE_PARM_DESC(arr, "Integer array, have to be populated");

static char string[MAX_STRLEN];
module_param_string(s, string, MAX_STRLEN, 0);

struct dentry *debugfs_create(void);
struct dentry *db_parent;
extern u32 test_db;
extern CNTXT *db_struct;

CNTXT *test = NULL;
wait_queue_head_t myqueue;

static int minor_open(struct inode *inode, struct file *filp)
{
	int res = 0;

	test = kmalloc(sizeof(char) * sizeof(CNTXT), GFP_KERNEL);
	if (!test) {
		pr_err("Dynamic memory allocation failed\n");
		res = -1;
	} else{
		test->buf = str;
		test->arry = arr;
		test->len = strlen(str);
		test->windex = 0;
		test->rindex = 0;
		spin_lock_init(&test->lock);

		filp->private_data =
		kmalloc(sizeof(char) * sizeof(CNTXT), GFP_KERNEL);
		if (!filp->private_data) {
			pr_err("Dynamic memory allocation failed\n");
			res = -1;
		} else {
			memcpy(filp->private_data, test, sizeof(CNTXT));
		}
	}
	return res;
}

static ssize_t minor_write(struct file *filp, const char __user *buf,
	size_t count, loff_t *f_pos)
{
	unsigned long flags;

	spin_lock_irqsave(&test->lock, flags);

	copy_from_user(test->buf, buf, count);
	test->buf[count] = '\0';
	test->len = count;
	test->windex = count;

	*((CNTXT *) filp->private_data) = *((CNTXT *) test);
	*((CNTXT *) db_struct) = *((CNTXT *) test);

	spin_unlock_irqrestore(&test->lock, flags);

	atomic_set(&flag, 1);
	wake_up_interruptible(&myqueue);
	return count;
}

static ssize_t minor_read(struct file *filp, char __user *buf, size_t count,
	loff_t *f_pos)
{
	unsigned long flags;
	unsigned int rem = 0;
	if (count > test->len)
		count = test->len;

	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;

	if (0 == atomic_read(&flag))
		if (wait_event_interruptible(myqueue,
					(atomic_read(&flag) == 1)))
			return -ERESTARTSYS;

	spin_lock_irqsave(&test->lock, flags);

	test->rindex = count;

	*((CNTXT *) test) = *((CNTXT *) filp->private_data);
	test_db = test->rindex;

	pr_debug("Read Index:%u \t debug:%u remainder:%u\n", test->rindex,
		test_db, rem);

	rem = copy_to_user(buf, test->buf, count);

	spin_unlock_irqrestore(&test->lock, flags);

	return count;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long minor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int minor_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
	unsigned long arg)
#endif
{
	int ret = 0;
	int res = 0;

	if (_IOC_TYPE(cmd) != IOCTL_HELLO_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > IOCTL_MAX_CMDS)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		res = !access_ok(VERIFY_WRITE, (void __user *)arg,
						_IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		res = !access_ok(VERIFY_READ, (void __user *)arg,
						_IOC_SIZE(cmd));

	if (res)
		return -EFAULT;

	switch			(cmd) {
	case IOCTL_HELLO_TEST1:
				pr_info("In IOCTL test 1\n");
				break;

	case IOCTL_HELLO_TEST2:
				pr_info("In IOCTL test 2\n");
				break;

	default:
				pr_info("In IOCTL default case\n");
				break;
	}

	return ret;
}

static int minor_close(struct inode *inode, struct file *filp)
{
	int res = 0;
	int index = 0;
	CNTXT result;

	result = *((CNTXT *) filp->private_data);

#if 0
	dmsg("str:%s \t len:%d \t rindex:%d \t windex:%d\n",
			*((long *) filp->private_data),
			*(unsigned int *) (filp->private_data + 2),
			*(unsigned int *) (filp->private_data + 3),
			*(unsigned int *) (filp->private_data + 4));

	dmsg("len addr:%p \t rindex:%p\n", (long *)(filp->private_data + 2),
			(long *)(filp->private_data + 3));

dmsg("arr:%ld \t test:%ld \t arry:%ld\n", (long) arr, (long) test->arry,
	     *((long *) filp->private_data + 1));

	for (index = 0; index < MAX_LEN; index++)
		dmsg("arr[%d]: %d\n", index,
	*((unsigned int *)(*((long *)filp->private_data + 1)) + index));
#endif

	pr_debug("String:%s\t len:%u\t rindex:%u\t windex:%u\n", result.buf,
	result.len, result.rindex, result.windex);

	for (index = 0; index < MAX_LEN; index++)
		pr_debug("arr[%d]: %d\n", index, result.arry[index]);


	kfree(test);
	kfree(filp->private_data);
	kfree(db_struct);

	return res;
}

const struct file_operations hello_minor_fops = {
	.owner		= THIS_MODULE,
	.open		= minor_open,
	.release	= minor_close,
	.write		= minor_write,
	.read		= minor_read,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = minor_ioctl
#else
	.ioctl		  = minor_ioctl
#endif
};

static int hello_open(struct inode *inode, struct file *filp)
{
	int res = 0;

	pr_warn("Minor no specific file operations: %d\n", iminor(inode));

	switch (iminor(inode)) {
	case 0:
		filp->f_op = &hello_minor_fops;
		break;
	default:
		pr_warn("There is no minor no with this value : %d\n",
		iminor(inode));
		break;
	}

	if (filp->f_op && filp->f_op->open)
		res = filp->f_op->open(inode, filp);
	else
		res = 0;

	return res;
}

const struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open  = hello_open
};

static int __init hello_init_module(void){
	int major = 0;
	int res = SUCCESS;
	int index = 0;
	int len = 0;

	for (index = 0; index < MAX_LEN; index++)
		pr_debug("arr[%d]:%d\n", index, arr[index]);

	pr_debug("Module param str:%s\n", str);

	len = strlen(string);

	for (index = 0; index < len; index++)
		pr_debug("string[%d]:%c\n", index, string[index]);

	iline(string);

	major = register_chrdev(majorno, MODULE_NAME, &hello_fops);
	if (major < 0)
		pr_err("Failed to register hello driver\n");
	else{
		pr_info("Driver registered, majorno:%d\n", major);

		if (0 == majorno)
			majorno = major;

		init_waitqueue_head(&myqueue);

		db_parent = debugfs_create();
	}

	return res;
}

int test_export(void)
{
	int ret = 0;

	pr_info("Testing\n");
	ret = 1;

	return ret;
}
#if 0
EXPORT_SYMBOL(test_export);
#endif

static void __exit hello_cleanup_module(void)
{
	debugfs_remove_recursive(db_parent);

	unregister_chrdev(majorno, MODULE_NAME);
	pr_info("Exiting from the kernel\n");
}

module_init(hello_init_module);
module_exit(hello_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Hello driver module with debug information");
MODULE_VERSION(MOD_VERSION);
MODULE_AUTHOR("Gautham Kantharaju <gautham.kantharaju@gmail.com>");

