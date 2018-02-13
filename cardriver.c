#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define  DEVICE_NAME "carchardev"
#define  CLASS_NAME  "car"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Forbes Turley");
MODULE_DESCRIPTION("Monitors 4 GPIO pins as defined below for directional input");
MODULE_VERSION("1.0");

static int    majorNumber;
static int    numberInputs = 0;
static char data[256];
static struct class*  carcharClass  = NULL;
static struct device* carcharDevice = NULL;


static unsigned int gpioUp = 17;
static unsigned int gpioDown = 27;
static unsigned int gpioLeft = 22;
static unsigned int gpioRight = 23;

static int     dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static int     dev_release(struct inode *, struct file *);


static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.release = dev_release,
};


static int __init carchar_init(void){
	printk(KERN_INFO "CarChar: Initializing the CarChar LKM\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		printk(KERN_ALERT "CarChar failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "CarChar: registered correctly with major number %d\n", majorNumber);

	carcharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(carcharClass)){
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(carcharClass);
	}
	printk(KERN_INFO "CarChar: device class registered correctly\n");

	carcharDevice = device_create(carcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(carcharDevice)){
		class_destroy(carcharClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(carcharDevice);
	}
	printk(KERN_INFO "CarChar: device class created correctly\n");

	// Register all the GPIO pins
	gpio_request(gpioUp, "sysfs");
	gpio_request(gpioDown, "sysfs");
	gpio_request(gpioLeft, "sysfs");
	gpio_request(gpioRight, "sysfs");

	gpio_direction_input(gpioUp);
	gpio_direction_input(gpioDown);
	gpio_direction_input(gpioLeft);
	gpio_direction_input(gpioRight);

	gpio_export(gpioUp, 0);
	gpio_export(gpioDown, 0);
	gpio_export(gpioLeft, 0);
	gpio_export(gpioRight, 0);

	return 0;
}

static void __exit carchar_exit(void){
	device_destroy(carcharClass, MKDEV(majorNumber, 0));
	class_unregister(carcharClass);
	class_destroy(carcharClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);

	gpio_unexport(gpioUp);  
	gpio_unexport(gpioDown);  
	gpio_unexport(gpioLeft);  
	gpio_unexport(gpioRight);

	gpio_free(gpioUp);   
	gpio_free(gpioDown);   
	gpio_free(gpioLeft);   
	gpio_free(gpioRight); 

	printk(KERN_INFO "CarChar: Goodbye from the LKM!\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "CarChar: Device has been opened\n");
	return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    int output = 0;
	if(gpio_get_value(gpioUp)){
	    output = 1;
    }

	if(gpio_get_value(gpioDown)){
	    output = 2;
    }
	if(gpio_get_value(gpioLeft)){
	    output = 3;
    }
	if(gpio_get_value(gpioRight)){
	    output = 4;
    }
	int error_count = 0;
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
    return output;
}


static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "EBBChar: Device successfully closed\n");
   return 0;
}


module_init(carchar_init);
module_exit(carchar_exit);
