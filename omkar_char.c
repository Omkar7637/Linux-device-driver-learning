#include <linux/module.h>   // Required for all kernal modules
#include <linux/kernel.h>   // printk()
#include <linux/fs.h>       // file_operations structure 
#include <linux/access.h>   // copy_to_user() and cpoy_from_user()

// Name of the device taht will appera in /proc/devices
#define DEVICE_NAME "Omkar_Device"

// Class Name (Normally used when auto-creating /dev node via udev, not fully used here)
#define CLASS_NAME "Omkar_Class"

// Major number = ID given by kernal to your driver
// Kernal uses this number to know which driver to call
static int major;

// 