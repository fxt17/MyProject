#include <linux/module.h>

#include <linux/init.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>

/* 4. 寄存器物理地址与虚拟地址
 */
#define CCM_CCGR1_BASE				(0X020C406C)/*时钟寄存器*/
#define IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03_BASE	(0X020E0068)/*GPIO1_IO03复用寄存器*/
#define IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03_BASE   (0X020E02F4)/*GPIO1_IO03属性寄存器*/
#define GPIO1_GDIR_BASE				(0X0209C004)/*GPIO1方向寄存器*/
#define GPIO1_DR_BASE				(0X0209C000)/*GPIO1数据寄存器*/

static void __iomem *CCM_CCGR1;
static void __iomem *IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03;
static void __iomem *IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03;
static void __iomem *GPIO1_GDIR;
static void __iomem *GPIO1_DR;

/* 7. 驱动操作函数的变量与宏定义
 */
static char kernel_buf[1];
#define MIN(a, b) (a < b ? a : b)

/* 6. 驱动操作函数
 */
static int led_drv_open (struct inode *node, struct file *file)
{
	printk("%s\r\n",__func__);
	return 0;
}

static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	u32 val;
	printk("%s\r\n",__func__);
	val = readl(GPIO1_DR);/*读取数据寄存器的值*/
	kernel_buf[0] = ((val & (1<<3)) >> 3);/*取bit[3]的值*/
	err = copy_to_user(buf, kernel_buf, MIN(1, size));/*内核数据传入用户空间*/
	if(err)
        {
                printk("%s:failed!\r\n",__func__);
                return -EFAULT;
        }
	return MIN(1, size);
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	long val, data;
	printk("%s\r\n",__func__);
	err = copy_from_user(kernel_buf, buf, MIN(1, size));/*用户数据传入内核空间*/
	if(err)
	{
		printk("%s:failed!\r\n",__func__);
		return -EFAULT;
	}
	if (kstrtol(kernel_buf, 10, &val) < 0) 
	{
		printk("kstrtol 失败\r\n");
		return -EINVAL;
	}
	printk("用户写入数据为:%ld\r\n", val);
	data = readl(GPIO1_DR);/*获取寄存器原本的值*/
	val = ((data & (~(1 << 3))) | (val << 3));
	writel(val, GPIO1_DR);/*用户传入的参数直接写入数据寄存器bit[3]*/
	printk("内核写入寄存器值为:%#lx\r\n", val);
	return MIN(1, size);
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s\r\n",__func__);
	return 0;
}

/* 5. 注册/卸载函数的变量与宏定义 
 */
struct drv_attribute {
	dev_t devid;/*设备号，MAJOR(devid)为主设备号，MINOR(devid)为次设备号*/
	int major;/*主设备号*/
	const char *drv_name;/*设备名称*/
	struct cdev cdev;/**/
	struct class *class;/*设备类*/
	const char *class_name;/*类名称*/
	struct device *device;/*设备*/
};
static struct drv_attribute led_drv_attribute = {
	.cdev.owner	= THIS_MODULE,
	.drv_name	= "led_drv",
	.class_name	= "led_drv_class",
};
static struct file_operations led_drv_fops = {
	.owner	= THIS_MODULE,
	.open	= led_drv_open,
	.read	= led_drv_read,
	.write	= led_drv_write,
	.release= led_drv_close,
};

/* 2. 驱动注册函数
 */
static int __init led_drv_init(void)
{
	int val;
	int ret;
	/*寄存器映射*/
	CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,0x1000);
	IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 = ioremap(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03_BASE,0x1000);
	IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03 = ioremap(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03_BASE,0x1000);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE,0x1000);
	GPIO1_DR = ioremap(GPIO1_DR_BASE,0x1000);
	/*LED初始化*/
	val = readl(CCM_CCGR1);/*获取原本的时钟设置*/
	val &= ~(3 << 26);/*清除bit[27:26]以前的设置*/
	val |= (3 << 26);/*设置bit[27:26]新值*/
	writel(val, CCM_CCGR1);/*打开GPIO1时钟*/
	writel(5, IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03);/*IO复用为GPIO1_IO03*/
	writel(0x10B0, IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03);/*设置为常用的输出配置*/
	val = readl(GPIO1_GDIR);/*获取原本的GPIO方向设置*/
	val &= ~(1 << 3);/*清除bit[3]以前的设置*/
	val |= (1 << 3);/*设置bit[3]新值*/
	writel(val, GPIO1_GDIR);/*设置GPIO1_IO03为输出*/
	/*自动获取设备号并注册设备号*/
	if(led_drv_attribute.major)
	{
		led_drv_attribute.devid = MKDEV(led_drv_attribute.major, 0);/*设置次设备号0，构建完整的设备号*/
		register_chrdev_region(led_drv_attribute.devid, 1, led_drv_attribute.drv_name);/*注册设备号*/
	}
	else
	{
		alloc_chrdev_region(&led_drv_attribute.devid, 0, 1, led_drv_attribute.drv_name);/*自动获取并注册设备号*/
	}
	/*初始化结构体struct cdev变量，将操作函数file_operations、设备号dev_t等信息存入struct cdev中*/
	cdev_init(&led_drv_attribute.cdev, &led_drv_fops);
	/*注册设备驱动*/
	ret = cdev_add(&led_drv_attribute.cdev, led_drv_attribute.devid, 1);
	if(ret==0)
	{
		printk("%s:驱动已注册...\r\n",__func__);
		printk("驱动主设备号:%d,次设备号:%d\r\n",MAJOR(led_drv_attribute.devid),MINOR(led_drv_attribute.devid));
	}
	else
	{
		printk("%s:驱动注册失败!\r\n",__func__);
		return -EFAULT;
	}
	/*创建名为class_name的class类*/
	led_drv_attribute.class = class_create(THIS_MODULE, led_drv_attribute.class_name);
	/*创建一个class类下的设备*/
	led_drv_attribute.device = device_create(led_drv_attribute.class, NULL, led_drv_attribute.devid, NULL, led_drv_attribute.drv_name);
	return 0;
}

/* 3. 驱动卸载函数
 */
static void __exit led_drv_exit(void)
{
	/*删除class类下的设备*/
	device_destroy(led_drv_attribute.class, led_drv_attribute.devid);
	/*删除class类*/
	class_destroy(led_drv_attribute.class);
	/*注销字符设备*/
	cdev_del(&led_drv_attribute.cdev);
	/*释放设备号*/
	unregister_chrdev_region(led_drv_attribute.devid, 1);
	/*取消寄存器映射*/
	iounmap(CCM_CCGR1);
	iounmap(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03);
	iounmap(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03);
	iounmap(GPIO1_GDIR);
	iounmap(GPIO1_DR);
	printk("%s:驱动已卸载\r\n",__func__);
}

/* 1. 注册驱动注册/卸载函数
 */
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");


