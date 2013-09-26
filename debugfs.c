#include "main.h"

struct dentry *parent = NULL;
u32 test_db = 0;
CNTXT *db_struct = NULL;
char wrbuf[sizeof(CNTXT) * 5];


static 
ssize_t db_read(struct file *filp, char __user *buf, size_t count,loff_t *fpos)
{
	CNTXT *data = (CNTXT *) filp->f_dentry->d_inode->i_private;
	char rdbuf[sizeof(CNTXT) * 5];

	snprintf(rdbuf,(sizeof(CNTXT) * 3), 
	         "Data: buf:%s \n arry:%ld \n len:%d \n rindex:%d \n windex:%d \n", 
	         data->buf, (long)data->arry, data->len, data->rindex, 
	         data->windex);

	return simple_read_from_buffer(buf, (sizeof(CNTXT) * 3), fpos, rdbuf, 
	                               (sizeof(CNTXT) * 3));
}

#if 0
static ssize_t db_write(struct file *filp, const char __user *buf, size_t count, loff_t *fpos)
{
	return simple_write_to_buffer(wrbuf, sizeof(CNTXT), fpos, buf, sizeof(CNTXT));
}
#endif

static struct file_operations db_fops = {
	.read = db_read,
};

#if 0
static struct file_operations db_fops1 = {
	.write = db_write,
};
#endif

struct dentry *debugfs_create(void)
{
	struct dentry *file = NULL;

	parent = debugfs_create_dir(MODULE_NAME, NULL);
	if(!parent)
	{
		dmsg(KERN_ALERT" Failed to create debugfs directory \n");
		parent = NULL;
	}
	
	else
	{
		file = debugfs_create_u32("db_test", 0666, parent, &test_db);
		if(!file)
		{
			dmsg(KERN_ALERT
			     "Failed to create debugfs file under parent directory \n");
			file = NULL;
		}	

		db_struct = (CNTXT *) kmalloc(sizeof(char) * sizeof(CNTXT),
		             GFP_KERNEL);
		if( !db_struct )
		{
			dmsg("Dynamic memory allocation failed \n");
			db_struct = NULL;
		}

		db_struct->buf = "Initial value";
		db_struct->arry = &test_db;
		db_struct->len = 1;
		db_struct->rindex = 1;
		db_struct->windex = 1;

		debugfs_create_file("db_struct", 0666, parent, db_struct, &db_fops);
	}

#if 0
	debugfs_create_file("db_struct1", 0666, parent, wrbuf, &db_fops1);
#endif

	return parent;
}
