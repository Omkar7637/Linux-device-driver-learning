#include <linux/module.h>   // Required for all kernal modules
#include <linux/kernel.h>   // printk()
#include <linux/fs.h>       // file_operations structure 
#include <linux/uaccess.h>   // copy_to_user() and cpoy_from_user()

// Name of the device taht will appera in /proc/devices
#define DEVICE_NAME "Omkar_Device"

// Class Name (Normally used when auto-creating /dev node via udev, not fully used here)
#define CLASS_NAME "Omkar_Class"

// Major number = ID given by kernal to your driver
// Kernal uses this number to know which driver to call
static int major;

// Kernal Buffer - This lives in KERNAL SPACE (NOT accessiable directly by user program)
static char message[256] = "Hello from kernal!";

// Size of valid data inside message []
static short message_size;

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

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int error_count;

    /*
        Very important function
        You cannot access directly access user memory in kernal.
        that wold crash the system.
        so kernal provides safe API:
        copy_tp_user(destination_user, source_kernal, size)
    */
   error_count = copy_to_user(buffer, message, message_size);

   if(error_count == 0)
   {
        printk(KERN_INFO "Omkar Device: Sent %d charachters to user\n", message_size);
        return message_size; // Number of bytes sent to user
   }
   else
   {
    printk(KERN_INFO "Omkar Device: Failed to send\n");
    return -EFAULT; // bad memory access
   }
}

/*
    Called when user does:
        write(fd, buffer, len);

    user send data to kernal
*/
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    sprintf(message, "%s", buffer);
    message_size = strlen(message);

    printk(KERN_INFO "Omkar Device: Recived %zu characters from user\n", len);
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
    /*
        register_chardev
        Registers your driver with kernal
        0 means:
        "Kernal please dynamically assign major number"
    */
   major = register_chrdev(0, DEVICE_NAME, &fops);

   printk(KERN_INFO "Omkar Device registered with major number %d\n", major);

   return 0;
}

// Module unload function
static void __exit char_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Omkar Device: unregistered\n");
}

module_init(char_init);
module_exit(char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Omkar Kashid");
MODULE_DESCRIPTION("Simple character Driver");