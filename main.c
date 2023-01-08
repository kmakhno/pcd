#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kostiantyn Makhno");
MODULE_DESCRIPTION("Simple pseudo character driver");

#define DEV_MEM_SIZE 512
struct pcd_dev {
	struct cdev pcd_device;
	dev_t dev_num;
	char dev_buff[DEV_MEM_SIZE];
};

static struct pcd_dev *dev;
static struct class *class_pcd;
static struct device *device_pcd;

loff_t pcd_llseek(struct file *filp, loff_t pos, int whence)
{
	switch (whence) {

	case SEEK_SET:
		filp->f_pos = pos;
		break;

	case SEEK_CUR:
		filp->f_pos += pos;
		break;

	case SEEK_END:
		filp->f_pos = DEV_MEM_SIZE + pos;
		break;

	default:
		return -EINVAL;
	}

	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *ubuf,
		size_t count, loff_t *pos)
{
	long not_copied;

	pr_info("read requested bytes %zu\n", count);
	if (*pos + count > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *pos;

	not_copied = copy_to_user(ubuf, &dev->dev_buff[*pos], count);
	count -= not_copied;

	pr_info("read %zu bytes\n", count);

	*pos += count;

	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *ubuf,
		size_t count, loff_t *pos)
{
	long not_copied;

	pr_info("write requested bytes %zu\n", count);
	if (*pos + count > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *pos;

	if (!count)
		return -ENOMEM;

	not_copied = copy_from_user(&dev->dev_buff[*pos], ubuf, count);
	count -= not_copied;

	pr_info("written %zu bytes\n", count);

	*pos += count;

	return count;
}

int pcd_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
	return 0;
}

const struct file_operations pcd_fops = {
	.owner = THIS_MODULE,
	.open = pcd_open,
	.release = pcd_release,
	.read = pcd_read,
	.write = pcd_write,
	.llseek = pcd_llseek
};

static int __init pcd_init(void)
{
	int ret;

	ret = -ENOMEM;
	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		goto out_mem;

	ret = alloc_chrdev_region(&dev->dev_num, 0, 1, "pcd_devices");
	if (ret < 0)
		goto out_acr;

	cdev_init(&dev->pcd_device, &pcd_fops);
	dev->pcd_device.owner = THIS_MODULE;

	ret = cdev_add(&dev->pcd_device, dev->dev_num, 1);
	if (ret < 0)
		goto out_cdev;

	pr_info("new device was allocated with major:minor %u:%u\n",
		MAJOR(dev->dev_num), MINOR(dev->dev_num));

	class_pcd = class_create(THIS_MODULE, "pcd_class");
	device_pcd = device_create(class_pcd, NULL, dev->dev_num, NULL, "pcd");

out_cdev:
out_acr:
out_mem:
	return ret;
}

static void __exit pcd_exit(void)
{
	device_destroy(class_pcd, dev->dev_num);
	class_destroy(class_pcd);
	cdev_del(&dev->pcd_device);
	unregister_chrdev_region(dev->dev_num, 1);
	kfree(dev);
}


module_init(pcd_init);
module_exit(pcd_exit);
