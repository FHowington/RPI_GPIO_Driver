#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

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

static unsigned int irqUp;
static unsigned int irqDown;
static unsigned int irqLeft;
static unsigned int irqRight;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static irq_handler_t  cargpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);
void debounceHandler(unsigned long data);

static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.release = dev_release,
};

int debounce_interval = 1;
struct timer_list timers;

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


	irqUp = gpio_to_irq(gpioUp);
	irqDown = gpio_to_irq(gpioDown);
	irqLeft = gpio_to_irq(gpioLeft);
	irqRight = gpio_to_irq(gpioRight);

	request_irq(irqUp,
			(irq_handler_t) cargpio_irq_handler,
			IRQF_TRIGGER_HIGH,
			"car_gpio_handler",
			NULL);
	request_irq(irqDown,
			(irq_handler_t) cargpio_irq_handler,
			IRQF_TRIGGER_HIGH,
			"car_gpio_handler",
			NULL);
	request_irq(irqLeft,
			(irq_handler_t) cargpio_irq_handler,
			IRQF_TRIGGER_HIGH,
			"car_gpio_handler",
			NULL);

	request_irq(irqRight,
			(irq_handler_t) cargpio_irq_handler,
			IRQF_TRIGGER_HIGH,
			"car_gpio_handler",
			NULL);

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

	free_irq(irqUp, NULL); 
	free_irq(irqDown, NULL); 
	free_irq(irqLeft, NULL); 
	free_irq(irqRight, NULL); 

	printk(KERN_INFO "CarChar: Goodbye from the LKM!\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "CarChar: Device has been opened\n");
	return 0;
}

static int dev_release(struct inode *inodep, struct file *filep){
	printk(KERN_INFO "CarChar: Device has been released\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){

	int error_count = 0;
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success

	if(len > numberInputs)
		len = numberInputs;
	error_count = copy_to_user(buffer, data, len);

	if (error_count==0){        
		numberInputs-=len;    // if true then have success
		return (len);  // clear the position to the start and return 0
	}

	else {
		printk(KERN_INFO "Car Character Driver: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;             
	}
}


static irq_handler_t cargpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
	if(irq == irqUp && numberInputs < 256){
		disable_irq_nosync(irqUp);
		data[numberInputs++] = 'u';
		init_timer(&timers);
		timers.expires = jiffies + debounce_interval;
		timers.data = 0;
		timers.function = debounceHandler;
		add_timer(&timers);	
	}

	if(irq == irqDown && numberInputs < 256){
		disable_irq_nosync(irqDown);
		data[numberInputs++] = 'd';
		init_timer(&timers);
		timers.expires = jiffies + debounce_interval;
		timers.data = 1;
		timers.function = debounceHandler;
		add_timer(&timers);	
	}

	if(irq == irqLeft && numberInputs < 256){
		disable_irq_nosync(irqLeft);
		data[numberInputs++] = 'l';
		init_timer(&timers);
		timers.expires = jiffies + debounce_interval;
		timers.data = 2;
		timers.function = debounceHandler;
		add_timer(&timers);	
	}

	if(irq == irqRight && numberInputs < 256){
		disable_irq_nosync(irqRight);
		data[numberInputs++] = 'r';
		init_timer(&timers);
		timers.expires = jiffies + debounce_interval;
		timers.data = 3;
		timers.function = debounceHandler;
		add_timer(&timers);	
	}

	return (irq_handler_t) IRQ_HANDLED;
}


void debounceHandler(unsigned long data)
{
	if(data == 0)
		enable_irq(irqUp);


	if(data == 1)
		enable_irq(irqDown);

	if(data == 2)
		enable_irq(irqLeft);

	if(data == 3)
		enable_irq(irqRight);

	printk(KERN_INFO "Timer Handler called.\n");
}


module_init(carchar_init);
module_exit(carchar_exit);
