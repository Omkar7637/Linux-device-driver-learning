/*
    omkar_char.c
    
    A Simple Linux Character Device Driver
    
    This driver demonstrates communication between user space and 
    kernal space using a character device.The device allows a user
    application to open, read, write and close a device file located
    in /dev/Omkar_Device.

    The driver automatically creates the device node using the 
    Linux device model (Class_create + create_create) and does not
    require manual mknod.

    Author  :   Omkar Kashid
    License :   GPL
*/


#include <linux/module.h>   // Core header for loading LKMs
#include <linux/kernel.h>   // printk()
#include <linux/fs.h>       // file_operations, register_chrdev 
#include <linux/uaccess.h>  // copy_to_user() and cpoy_from_user()
#include <linux/string.h>   // strlen, memset
#include <linux/init.h>     // __init, __exit macros
#include <linux/device.h>   // class_create, device_create

// Device name visible in /dev and /proc/devices
#define DEVICE_NAME "Omkar_Device"

// Device class name visible in /sys/class
#define CLASS_NAME "Omkar_Class"

// Major number assigned by kernal during registration
static int major;

/*
    Kernal buffer
    Stores data recived from user space and later

*/
static char message[256] = "Hello from kernal!";

// Size of valid data inside message []
static short message_size;

static struct class*  omkarClass  = NULL;
static struct device* omkarDevice = NULL;

/*
    Called when user does:
        Open("/dev/Omkar_device")

    inodep  -> Information about device file
    filep   -> file instanece created by kernal
*/

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Omkar Device: Opened\n");
    return 0;
}

/*
    Called when user does:
        read(fd, buffer, len);
    
    filep   -> file structure
    buffer  -> USER SPACE buffer (IMPORTANT!!!)
    len     -> how many bytes user wants
    offset  -> File position pointer
*/

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
    int bytes_to_copy;

    if (*offset >= message_size)
        return 0;

    bytes_to_copy = min(len, (size_t)(message_size - *offset));

    if (copy_to_user(buffer, message + *offset, bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;

    printk(KERN_INFO "Omkar Device: sent %d bytes\n", bytes_to_copy);
    return bytes_to_copy;
}

/*
    Called when user does:
        write(fd, buffer, len);

    user send data to kernal
*/
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    if (len > sizeof(message) - 1)
        len = sizeof(message) - 1;

    if (copy_from_user(message, buffer, len))
        return -EFAULT;

    message[len] = '\0';
    message_size = len;

    printk(KERN_INFO "Omkar Device: Received %zu characters from user\n", len);
    return len;
}

// Called when user does: 
//      close(fd);

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Omkar Device: Closed\n");
    return 0;
}

/*
    This structure CONNECTS system calles to functions
    WHen user calls read() -> Kernal calls dev_read()
    when user calls write() -> kernal calls dev_write()
*/

static struct file_operations fops = 
{
    .open = dev_open, 
    .read = dev_read, 
    .write = dev_write, 
    .release = dev_release,
};

// Module load function
static int __init char_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Omkar: failed to register major\n");
        return major;
    }

    omkarClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(omkarClass)) {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(omkarClass);
    }

    omkarDevice = device_create(omkarClass, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(omkarDevice)) {
        class_destroy(omkarClass);
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create device\n");
        return PTR_ERR(omkarDevice);
    }

    printk(KERN_INFO "Omkar Device: created correctly\n");
    return 0;
}

// Module unload function
static void __exit char_exit(void)
{
    device_destroy(omkarClass, MKDEV(major, 0));
    class_unregister(omkarClass);
    class_destroy(omkarClass);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Omkar Device: removed\n");
}

module_init(char_init);
module_exit(char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Omkar Kashid");
MODULE_DESCRIPTION("Simple character Driver");