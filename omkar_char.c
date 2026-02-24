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
    Stores data recived from user space and later returned on read()
    This memory resides in kernal space and cannot be accessed directly
    by user application.
*/
static char message[256] = "Hello from kernal!";

// Tracks numbe of valid bytes currently stored in message[]
static short message_size;

// Device class and device structure pointers (Linux device model)
static struct class*  omkarClass  = NULL;
static struct device* omkarDevice = NULL;

/*----------------------------------------------------------------------*/
/*                          FILE OPERATIONS                             */
/*----------------------------------------------------------------------*/

/*
    dev_open - callled when user application opens the device file 
    Triggered by:
        open("/dev/Omkar_Device", ORDWR);

    @inodep :   Pointer to inode structure representing device file metadata
    @filep  :   Pointer to file structure representing opened instance

    Return  :   0 on success
*/
static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Omkar Device: Opened\n");
    return 0;
}

/*
    dev_read - transfer data from kernal to user space

    Triggered by:
        read(fd, buffer, length);

    @filep  :   Pointer to file structure
    @buffer :   User-Space buffer (must use copy_to_user)
    @len    :   Number of bytes requested by user
    @offset :   Current file position pointer

    Return  :
        >0  :   number of bytes read
        0   :   End of file
        <0  :   error code
*/

static ssize_t dev_read(struct file *filep,
                        char __user *buffer,
                        size_t len,
                        loff_t *offset)
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
    dev_write - recivies data from user sapce

    Triggered by:
        write(fd, buffer, length);
        echo "hello" > /dev/Omkar_Device

    @filep  :   File structure
    @buffer :   User-space buffer (must NOT be accessed directly)
    @len    :   Number of bytes written
    @offset :   File 
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