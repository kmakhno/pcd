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

#define NO_DEVICES    4
#define DEV1_MEM_SIZE 1024
#define DEV2_MEM_SIZE 512
#define DEV3_MEM_SIZE 256
#define DEV4_MEM_SIZE 512

#define RDONLY 0x10
#define WRONLY 0x20
#define RDWR   0x30

char dev1_buffer[DEV1_MEM_SIZE];
char dev2_buffer[DEV2_MEM_SIZE];
char dev3_buffer[DEV3_MEM_SIZE];
char dev4_buffer[DEV4_MEM_SIZE];

struct pcd_dev {
	struct cdev pcd_device;
	char *buff;
	int perm;
	size_t size;
	const char *serial_number;
};

struct pcdrv_private_data {
	int total_devices;
	dev_t dev_num;
	struct class *class_pcd;
	struct device *device_pcd;
	struct pcd_dev pcd_dev_data[NO_DEVICES];
};

static struct pcdrv_private_data drv_data = {
	.total_devices = NO_DEVICES,
	.pcd_dev_data = {
		[0] = {
			.buff = dev1_buffer,
			.size = DEV1_MEM_SIZE,
			.serial_number = "pcd1",
			.perm = RDONLY
		},

		[1] = {
			.buff = dev2_buffer,
			.size = DEV2_MEM_SIZE,
			.serial_number = "pcd2",
			.perm = WRONLY
		},

		[2] = {
			.buff = dev3_buffer,
			.size = DEV3_MEM_SIZE,
			.serial_number = "pcd3",
			.perm = RDWR
		},

		[3] = {
			.buff = dev4_buffer,
			.size = DEV4_MEM_SIZE,
			.serial_number = "pcd4",
			.perm = RDWR
		}
	}
};

loff_t pcd_llseek(struct file *filp, loff_t pos, int whence)
{
	loff_t temp;

	switch (whence) {

	case SEEK_SET:
		if (pos > DEV_MEM_SIZE || pos < 0)
			return -EINVAL;
		filp->f_pos = pos;
		break;

	case SEEK_CUR:
		temp = filp->f_pos + pos;
		if (temp > DEV_MEM_SIZE || temp < 0)
			return -EINVAL;
		filp->f_pos = temp;
		break;

	case SEEK_END:
		if (pos > 0)
			return -EINVAL;
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
	int i;

	ret = alloc_chrdev_region(&drv_data.dev_num, 0, NO_DEVICES, "pcd_devices");
	if (ret < 0)
		goto err_alloc_dev_num;

	drv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
	if (IS_ERR(drv_data.class_pcd)) {
		ret = PTR_ERR(drv_data.class_pcd);
		goto err_class_create;
	}

	for (i = 0; i < NO_DEVICES; ++i) {
		pr_info("new device number was allocated: major:minor %u:%u\n",
			MAJOR(drv_data.dev_num + i), MINOR(drv_data.dev_num + i));

		cdev_init(&drv_data.pcd_dev_data[i].pcd_device, &pcd_fops);
		drv_data.pcd_dev_data[i].pcd_device.owner = THIS_MODULE;

		ret = cdev_add(&drv_data.pcd_dev_data[i].pcd_device, drv_data.dev_num + i, 1);
		if (ret < 0)
			goto err_cdev_reg;

		drv_data.device_pcd = device_create(drv_data.class_pcd, NULL, drv_data.dev_num + i, NULL, "pcd-%d", i);
		if (IS_ERR(drv_data.device_pcd)) {
			ret = PTR_ERR(drv_data.device_pcd);
			goto err_dev_create;
		}
	}

	return 0;

err_dev_create:
err_cdev_reg:
	for (i -= 1; i > 0; --i) {
		device_destroy(drv_data.class_pcd, drv_data.dev_num + i);
		cdev_del(&drv_data.pcd_dev_data[i].pcd_device);
	}
	class_destroy(drv_data.class_pcd);
err_class_create:
	unregister_chrdev_region(drv_data.dev_num, NO_DEVICES);
err_alloc_dev_num:
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
