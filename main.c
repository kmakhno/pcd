#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kostiantyn Makhno");
MODULE_DESCRIPTION("Simple pseudo character driver");

#define DEV_MEM_SIZE 512
struct pcd_dev {
	dev_t dev_num;
	char dev_buff[DEV_MEM_SIZE];
};

static struct pcd_dev *dev;

static int __init pcd_init(void)
{
	int ret;

	ret = -ENOMEM;
	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		goto out_mem;

	ret = alloc_chrdev_region(&dev->dev_num, 0, 1, "pcd");
	if (ret < 0)
		goto out_acr;

	pr_info("new device was allocated with major:minor %u:%u\n", 
	        MAJOR(dev->dev_num), MINOR(dev->dev_num));

out_acr:
out_mem:
	return ret;
}

static void __exit pcd_exit(void)
{
	unregister_chrdev_region(dev->dev_num, 1);
	kfree(dev);
}


module_init(pcd_init);
module_exit(pcd_exit);
