#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define DRIVER_NAME  "CDD_GPIO"
#define SAMPLE_PERIOD (HZ)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("noTengoGrupo");
MODULE_DESCRIPTION("CDD TP5 SdeC");

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;
static int selected_signal = 1;
static int fake_value = 0;
static struct timer_list my_timer;

static void timer_callback(struct timer_list *t)
{
    fake_value = 1 - fake_value;
    mod_timer(&my_timer, jiffies + SAMPLE_PERIOD);
}

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("CDD_GPIO: abierto\n");
    return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
    pr_info("CDD_GPIO: cerrado\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf,
                        size_t count, loff_t *offset)
{
    char kbuf[32];
    int len;
    if (*offset > 0) return 0;
    len = snprintf(kbuf, sizeof(kbuf), "%d\n", fake_value);
    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;
    *offset += len;
    return len;
}

static ssize_t my_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *offset)
{
    char kbuf[8];
    int new_signal;
    if (count > sizeof(kbuf) - 1) return -EINVAL;
    if (copy_from_user(kbuf, buf, count)) return -EFAULT;
    kbuf[count] = '\0';
    if (kstrtoint(kbuf, 10, &new_signal)) return -EINVAL;
    if (new_signal != 1 && new_signal != 2) return -EINVAL;
    selected_signal = new_signal;
    fake_value = 0;
    pr_info("CDD_GPIO: señal %d seleccionada\n", selected_signal);
    return count;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_close,
    .read    = my_read,
    .write   = my_write,
};

static int __init cdd_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&dev_num, 0, 1, DRIVER_NAME);
    if (ret < 0) { pr_err("CDD_GPIO: error major\n"); return ret; }

    my_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }
    my_device = device_create(my_class, NULL, dev_num, NULL, DRIVER_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        device_destroy(my_class, dev_num);
        class_destroy(my_class);
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + SAMPLE_PERIOD);
    pr_info("CDD_GPIO: cargado OK. Major=%d\n", MAJOR(dev_num));
    return 0;
}

static void __exit cdd_exit(void)
{
    del_timer_sync(&my_timer);
    cdev_del(&my_cdev);
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    unregister_chrdev_region(dev_num, 1);
    pr_info("CDD_GPIO: descargado\n");
}

module_init(cdd_init);
module_exit(cdd_exit);
