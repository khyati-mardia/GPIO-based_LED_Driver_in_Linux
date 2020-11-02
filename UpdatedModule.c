/* ----------------------------------------------- DRIVER gmem --------------------------------------------------

 Basic driver example to show skelton methods for several file operations.

 ----------------------------------------------------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
//#include <linux/jiffies.h>
#include <linux/gpio.h>
#include<linux/init.h>
#include<linux/moduleparam.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#define IOC_MAGIC 'k' // defines the magic number
#define SEQ_NR_LOCAL '0'
struct PinMapping {
	int gpioPin1;
	int gpioPin2;
	int gpioPin3;
	int gpioPin4;

}pinMappings[14]; // Defining the Pin Mapping structure.
struct RGBLED {
	int dutyCycle;
	int io1;
	int io2;
	int io3;
}rgbLed1;   // Input structure received from the user.
#define RGB_CONFIG _IOW(IOC_MAGIC, SEQ_NR_LOCAL, struct RGBLED) // Defin
int flag= 0;
/* per device structure */
struct rgb_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	char in_string[256];			/* buffer for the input string */
	int commandFromUser;			/*Taking command from user , which is 0,1,2,....7*/
	int current_write_pointer;
} *rgb_dev_1;

ktime_t on_time;							//On time structure
ktime_t off_time;							// Off time Structure
static struct hrtimer t1;					// Defining hrtimer instance
static dev_t rgb_device_number;      /* Allotted device number */
static struct class*  rgbcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* rgbcharDevice = NULL; ///< The device-driver device struct pointer
#define  CLASS_NAME  "rgb"        ///< The device class -- this is a character device driver
#define  DEVICE_NAME "RGBLed"				//Allocated Device Name.
int gpioPinToBeOff[3];						// Defining an global array of pins that need to get turned off.
void configuringPins(void);
void initialisePinData(void);
void switchOnLedSequence(void);
void timingFunction(void);

/**
 * Open function.
 */
int rgb_driver_open(struct inode *inode, struct file *file)
{
	printk("\n Entering UpdatedModule::rgb_driver_open method\n ");
	rgb_dev_1 = container_of(inode->i_cdev, struct rgb_dev, cdev); //Get the per-device structure that contains this cdev
	file->private_data = rgb_dev_1;  //Easy access to cmos_devp from rest of the entry points
	printk("\n %s is openning \n", rgb_dev_1->name);
	printk("\n Exiting UpdatedModule::rgb_driver_open method\n ");
	return 0;
}

/**
 *	Write function of driver.
 */
ssize_t rgb_driver_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk("\n Entering rgb_driver_write method :: UpdatedModule.c \n");
	printk(" \n in here-------------- \n ");
	rgb_dev_1 = file->private_data;
	while (count)
	{
		get_user(rgb_dev_1->in_string[rgb_dev_1->current_write_pointer], buf++); // reading the interger byte by byte
		count--;
		if(count)
		{
			rgb_dev_1->current_write_pointer++;
			if( rgb_dev_1->current_write_pointer == 256)
				rgb_dev_1->current_write_pointer = 0; //Overwriting the data already written by reading from 0 again.
		}
	}
	rgb_dev_1->current_write_pointer=0; // setting the pointer back to the location to write new value
	printk("\n Writing -- %d %s \n", rgb_dev_1->current_write_pointer, rgb_dev_1->in_string);
	kstrtoint(rgb_dev_1->in_string,10,&rgb_dev_1->commandFromUser);
	printk(" \n The command user sent is %d \n " , rgb_dev_1->commandFromUser);
	switchOnLedSequence();
	return 0;
}

//IOCTL Method.
long rgb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("\n Entering UpdatedModule::rgb_ioctl method \n");
	switch(cmd) {
	case RGB_CONFIG: {
		printk("\n IOCTL Command is RGB_CONFIG \n");
		if (copy_from_user(&rgbLed1, (void *)arg, sizeof(rgbLed1)))
		{
			return -EACCES;
		}
		printk("\n %d \n" , rgbLed1.dutyCycle);
		printk("\n %d \n" , rgbLed1.io1);
		printk("\n %d \n" , rgbLed1.io2);
		printk("\n %d \n" , rgbLed1.io3);
		printk(KERN_INFO "\n Pins Initialised in Kernel with duty cycle = %d \n",
				rgbLed1.dutyCycle);
		printk(KERN_INFO "\n IO Pin for Red is Initialised with value of = %d \n",
				rgbLed1.io1);
		printk(KERN_INFO "\n IO Pin for Green is Initialised with value of = %d \n",
				rgbLed1.io2);
		printk(KERN_INFO "\n IO Pin for BLue is Initialised with value of = %d \n",
				rgbLed1.io3);
		initialisePinData();
		configuringPins();
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin2, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin3, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin4, 0);

		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin2, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin3, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin4, 0);

		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin2, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin3, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin4, 0);
		break;
	}
	}
	printk("\n Exiting UpdatedModule::rgb_ioctl method \n");
	return 0;
}
/**
 * Configuring pins for switching action.
 */
void configuringPins() {
	if (pinMappings[rgbLed1.io1].gpioPin1 != 81) {
		printk("\n GPIO Request command :: Red :: gpio "
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin1);
		gpio_request(pinMappings[rgbLed1.io1].gpioPin1, "redLed");
		gpio_direction_output(pinMappings[rgbLed1.io1].gpioPin1, 1);

	}
	if (pinMappings[rgbLed1.io1].gpioPin2 != 81) {
		printk("\n GPIO Request command :: Red :: gpio"
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin2);
		gpio_request(pinMappings[rgbLed1.io1].gpioPin2, "redLed");
		gpio_direction_output(pinMappings[rgbLed1.io1].gpioPin2, 1);
	}
	if (pinMappings[rgbLed1.io1].gpioPin3 != 81) {
		printk("\n GPIO Request command :: Red :: gpio "
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin3);
		gpio_request(pinMappings[rgbLed1.io1].gpioPin3, "redLed");
		gpio_direction_output(pinMappings[rgbLed1.io1].gpioPin3, 1);
	}
	if (pinMappings[rgbLed1.io1].gpioPin4  != 81) {
		printk("\n GPIO Request command :: Red :: gpio "
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin4);
		gpio_request(pinMappings[rgbLed1.io1].gpioPin4, "redLed");
		gpio_direction_output(pinMappings[rgbLed1.io1].gpioPin4, 1);
	}
	if (pinMappings[rgbLed1.io2].gpioPin1 != 81) {
		printk("\n GPIO Request command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin1);
		gpio_request(pinMappings[rgbLed1.io2].gpioPin1, "greenLed");
		gpio_direction_output(pinMappings[rgbLed1.io2].gpioPin1, 1);

	}
	if (pinMappings[rgbLed1.io2].gpioPin2 != 81) {
		printk("\n GPIO Request command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin2);
		gpio_request(pinMappings[rgbLed1.io2].gpioPin2, "greenLed");
		gpio_direction_output(pinMappings[rgbLed1.io2].gpioPin2, 1);
	}
	if (pinMappings[rgbLed1.io2].gpioPin3 != 81) {
		printk("\n GPIO Request command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin3);
		gpio_request(pinMappings[rgbLed1.io2].gpioPin3, "greenLed");
		gpio_direction_output(pinMappings[rgbLed1.io2].gpioPin3, 1);
	}
	if (pinMappings[rgbLed1.io2].gpioPin4 != 81) {
		printk("\n GPIO Request command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin4);
		gpio_request(pinMappings[rgbLed1.io2].gpioPin4, "greenLed");
		gpio_direction_output(pinMappings[rgbLed1.io2].gpioPin4, 1);
	}
	if (pinMappings[rgbLed1.io3].gpioPin1 != 81) {
		printk("\n GPIO Request command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin1);
		gpio_request(pinMappings[rgbLed1.io3].gpioPin1, "BlueLed");
		gpio_direction_output(pinMappings[rgbLed1.io3].gpioPin1, 1);

	}
	if (pinMappings[rgbLed1.io3].gpioPin2 != 81) {
		printk("\n GPIO Request command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin2);
		gpio_request(pinMappings[rgbLed1.io3].gpioPin2, "BlueLed");
		gpio_direction_output(pinMappings[rgbLed1.io3].gpioPin2, 1);
	}
	if (pinMappings[rgbLed1.io3].gpioPin3 != 81) {
		printk("\n GPIO Request command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin3);
		gpio_request(pinMappings[rgbLed1.io3].gpioPin3, "BlueLed");
		gpio_direction_output(pinMappings[rgbLed1.io3].gpioPin3, 1);
	}
	if (pinMappings[rgbLed1.io3].gpioPin4 != 81) {
		printk("\n GPIO Request command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin4);
		gpio_request(pinMappings[rgbLed1.io3].gpioPin4, "BlueLed");
		gpio_direction_output(pinMappings[rgbLed1.io3].gpioPin4, 1);
	}
}
/**
 * Pin Matrix data.
 */
void initialisePinData() {
	pinMappings[0].gpioPin1 = 11;
	pinMappings[0].gpioPin2 = 32;
	pinMappings[0].gpioPin3 = 81;
	pinMappings[0].gpioPin4 = 81;

	pinMappings[1].gpioPin1 = 12;
	pinMappings[1].gpioPin2 = 28;
	pinMappings[1].gpioPin3 = 45;
	pinMappings[1].gpioPin4 = 81;

	pinMappings[2].gpioPin1 = 13;
	pinMappings[2].gpioPin2 = 34;
	pinMappings[2].gpioPin3 = 77;
	pinMappings[2].gpioPin4 = 81;

	pinMappings[3].gpioPin1 = 14;
	pinMappings[3].gpioPin2 = 16;
	pinMappings[3].gpioPin3 = 76;
	pinMappings[3].gpioPin4 = 64;

	pinMappings[4].gpioPin1 = 6;
	pinMappings[4].gpioPin2 = 36;
	pinMappings[4].gpioPin3 = 81;
	pinMappings[4].gpioPin4 = 81;

	pinMappings[5].gpioPin1 = 0;
	pinMappings[5].gpioPin2 = 18;
	pinMappings[5].gpioPin3 = 66;
	pinMappings[5].gpioPin4 = 81;

	pinMappings[6].gpioPin1 = 1;
	pinMappings[6].gpioPin2 = 20;
	pinMappings[6].gpioPin3 = 68;
	pinMappings[6].gpioPin4 = 81;

	pinMappings[7].gpioPin1 = 38;
	pinMappings[7].gpioPin2 = 81;
	pinMappings[7].gpioPin3 = 81;
	pinMappings[7].gpioPin4 = 81;

	pinMappings[8].gpioPin1 = 40;
	pinMappings[8].gpioPin2 = 81;
	pinMappings[8].gpioPin3 = 81;
	pinMappings[8].gpioPin4 = 81;

	pinMappings[9].gpioPin1 = 4;
	pinMappings[9].gpioPin2 = 22;
	pinMappings[9].gpioPin3 = 70;
	pinMappings[9].gpioPin4 = 81;

	pinMappings[10].gpioPin1 = 10;
	pinMappings[10].gpioPin2 = 26;
	pinMappings[10].gpioPin3 = 74;
	pinMappings[10].gpioPin4 = 81;

	pinMappings[11].gpioPin1 = 5;
	pinMappings[11].gpioPin2 = 24;
	pinMappings[11].gpioPin3 = 44;
	pinMappings[11].gpioPin4 = 72;

	pinMappings[12].gpioPin1 = 15;
	pinMappings[12].gpioPin2 = 42;
	pinMappings[12].gpioPin3 = 81;
	pinMappings[12].gpioPin4 = 81;

	pinMappings[13].gpioPin1 = 7;
	pinMappings[13].gpioPin2 = 30;
	pinMappings[13].gpioPin3 = 46;
	pinMappings[13].gpioPin4 = 81;

}
/**
 * Upon Ctrl+C from user, upon file close operation, the release function will be invoked.
 */
int rgb_driver_release(struct inode *inode, struct file *file)
{
	struct rgb_dev*rgb_dev_1 = file->private_data;
	if (pinMappings[rgbLed1.io1].gpioPin1 != 81) {
		printk("\n GPIO Free command :: Red :: gpio "
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin1);
		gpio_free(pinMappings[rgbLed1.io1].gpioPin1);

	}
	if (pinMappings[rgbLed1.io1].gpioPin2 != 81) {
		printk("\n GPIO Free command :: Red :: gpio"
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin2);
		gpio_free(pinMappings[rgbLed1.io1].gpioPin2);

	}
	if (pinMappings[rgbLed1.io1].gpioPin3 != 81) {
		printk("\n GPIO Free command :: Red :: gpio "
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin3);
		gpio_free(pinMappings[rgbLed1.io1].gpioPin3);

	}
	if (pinMappings[rgbLed1.io1].gpioPin4  != 81) {
		printk("\n GPIO Free command :: Red :: gpio "
				"%d \n" , pinMappings[rgbLed1.io1].gpioPin4);
		gpio_free(pinMappings[rgbLed1.io1].gpioPin4);
	}
	if (pinMappings[rgbLed1.io2].gpioPin1 != 81) {
		printk("\n GPIO Free command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin1);
		gpio_free(pinMappings[rgbLed1.io2].gpioPin1);

	}
	if (pinMappings[rgbLed1.io2].gpioPin2 != 81) {
		printk("\n GPIO Free command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin2);
		gpio_free(pinMappings[rgbLed1.io2].gpioPin2);

	}
	if (pinMappings[rgbLed1.io2].gpioPin3 != 81) {
		printk("\n GPIO Free command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin3);
		gpio_free(pinMappings[rgbLed1.io2].gpioPin3);

	}
	if (pinMappings[rgbLed1.io2].gpioPin4 != 81) {
		printk("\n GPIO Free command :: Green :: gpio"
				"%d \n" , pinMappings[rgbLed1.io2].gpioPin4);
		gpio_free(pinMappings[rgbLed1.io2].gpioPin4);
	}
	if (pinMappings[rgbLed1.io3].gpioPin1 != 81) {
		printk("\n GPIO Free command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin1);
		gpio_free(pinMappings[rgbLed1.io3].gpioPin1);

	}
	if (pinMappings[rgbLed1.io3].gpioPin2 != 81) {
		printk("\n GPIO Free command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin2);
		gpio_free(pinMappings[rgbLed1.io3].gpioPin2);
	}
	if (pinMappings[rgbLed1.io3].gpioPin3 != 81) {
		printk("\n GPIO Free command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin3);
		gpio_free(pinMappings[rgbLed1.io3].gpioPin3);
	}
	if (pinMappings[rgbLed1.io3].gpioPin4 != 81) {
		printk("\n GPIO Free command :: Blue :: gpio"
				"%d \n" , pinMappings[rgbLed1.io3].gpioPin4);
		gpio_free(pinMappings[rgbLed1.io3].gpioPin4);
	}
	printk(KERN_ALERT"\n%s is closing\n", rgb_dev_1->name);

	return 0;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations rgb_led_ops = {
		.owner		= THIS_MODULE,           /* Owner */
		.open		= rgb_driver_open,        /* Open method */
		.release	= rgb_driver_release,     /* Release method */
		.write		= rgb_driver_write,       /* Write method */
		.unlocked_ioctl		= rgb_ioctl,
};

/*
 * Driver Initialization
 */
int __init rgb_led_driver_init(void)
{

	int ret;
	printk("\n Entering HelloModule::rgb_led_driver_init method \n");
	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&rgb_device_number, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_DEBUG "Can't register device\n"); return -1;
	}
	printk("\n Allocating memory for rgb device \n");
	/* Populate sysfs entries */
	rgbcharClass = class_create(THIS_MODULE, DEVICE_NAME);
	printk("\n Creating rgb class structure \n");
	/* Allocate memory for the per-device structure */
	rgb_dev_1 = kmalloc(sizeof(struct rgb_dev), GFP_KERNEL);
	printk("\n Allocating memory for rgb class structure \n");
	if (!rgb_dev_1) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}
	printk("\n Initialising rgb led structure parameters \n");
	/* Request I/O region */
	sprintf(rgb_dev_1->name, DEVICE_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&rgb_dev_1->cdev, &rgb_led_ops);
	rgb_dev_1->cdev.owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&rgb_dev_1->cdev, (rgb_device_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	rgbcharDevice = device_create(rgbcharClass, NULL,
			MKDEV(MAJOR(rgb_device_number), 0), NULL, DEVICE_NAME);
	// device_create_file(gmem_dev_device, &dev_attr_xxx);

	memset(rgb_dev_1->in_string, 0, 256);

	rgb_dev_1->current_write_pointer = 0;

	printk("\n rgb led module initialized.\n");// '%s'\n",gmem_devp->in_string);
	printk("Exiting HelloModule::rgb_led_driver_init method \n ");
	return 0;
}


/* Driver Exit */
void __exit rgb_led_driver_exit(void)
{
	printk("Entering HelloModule::rgb_led_driver_exit method \n");
	unregister_chrdev_region((rgb_device_number), 1);
	printk("Unregistering character device \n");
	device_destroy (rgbcharClass, MKDEV(MAJOR(rgb_device_number), 0)); /* Destroy device */
	printk("Destroying device class of rgb \n");
	cdev_del(&rgb_dev_1->cdev);
	printk("Destroying cdev structure \n");
	kfree(rgb_dev_1);
	printk("Freeing memory of the allocated structures \n");
	/* Destroy driver_class */
	class_destroy(rgbcharClass);
	printk("Destroying memory of rgbclass structure \n");
	printk("Exiting HelloModule::rgb_led_driver_exit method \n");
}

/**
 *	Call_back_function.
 */
enum hrtimer_restart call_back_function(struct hrtimer *t2) {
	int i=0;
	if (flag == 0 ){
		//On state
		while(i<3) {
			if(gpioPinToBeOff[i] != 81) {
				gpio_set_value_cansleep(gpioPinToBeOff[i], 0); //Toggling from on to off.
			}
			i++;
		}
		flag = 1;
		hrtimer_forward(t2, ktime_get(),off_time); 			//Setting the new timer with new off time.
	}
	else if (flag == 1) {
		// Off state
		while(i<3) {
			if(gpioPinToBeOff[i] != 81) {
				gpio_set_value_cansleep(gpioPinToBeOff[i], 1); // Toggling from off to on.
			}
			i++;
		}
		flag = 0;
		hrtimer_forward(t2, ktime_get(), on_time);		//Setting the new timer with new off time.
	}
	return HRTIMER_RESTART;
}

void switchOnLedSequence() {
	int valueToLed = 1;
	printk("Entering switchOnLedSequence method in UpdatedModule.ko");
	switch (rgb_dev_1->commandFromUser) {
	case 0 : {
		printk("\n ALL LEDS ARE OFF \n");
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, 0);
		msleep(500);			//Time period of 0.5 seconds for every sequence.
		printk("\n Command of 0 executed , All leds are off \n");
		break;
	}
	case 1: {
		printk("\n RED LED IS ON \n");
		printk(" \n Command of 1 executed , Red led is on \n");
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, 0);

		gpioPinToBeOff[0] = pinMappings[rgbLed1.io1].gpioPin1;
		gpioPinToBeOff[1] = 81;
		gpioPinToBeOff[2] = 81;
		timingFunction();		//Invoking timer for every sequence.
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);			//Sleeping for 0.5 seconds.
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);	//Deleting timer.

		break;
	}
	case 2 : {
		printk("\n GREEN LED IS ON \n");
		printk(" \n Command of 2 executed , Green led is on \n");
		// Green led on
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, 0);

		printk(" \n Command of 1 executed , Green led is on \n");

		gpioPinToBeOff[0] = 81;
		gpioPinToBeOff[1] = pinMappings[rgbLed1.io2].gpioPin1;
		gpioPinToBeOff[2] = 81;
		timingFunction();
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);
		break;
	}

	case 4 : {
		printk("\n BLUE LED IS ON \n");
		printk("\n Command of 4 executed , Blue led is on \n");
		//Blue led on
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, valueToLed);


		gpioPinToBeOff[0] = 81;
		gpioPinToBeOff[1] = 81;
		gpioPinToBeOff[2] = pinMappings[rgbLed1.io3].gpioPin1;
		timingFunction();
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);

		break;
	}

	case 3 : {
		printk("\n Red and Green led IS ON \n");
		printk(" \n Command of 3 executed , Red and Green led is on \n");
		//Green and Red Led is on
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, 0);

		gpioPinToBeOff[0] = pinMappings[rgbLed1.io1].gpioPin1;
		gpioPinToBeOff[1] = pinMappings[rgbLed1.io2].gpioPin1;
		gpioPinToBeOff[2] = 81;

		timingFunction();
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);

		break;
	}

	case 6 : {
		printk("\n Blue and Green led IS ON \n");
		printk(" \n Command of 6 executed , Blue and Green led is on \n");
		//6 Blue and Green Led is on
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, valueToLed);

		gpioPinToBeOff[0] = 81;
		gpioPinToBeOff[1] = pinMappings[rgbLed1.io2].gpioPin1;
		gpioPinToBeOff[2] = pinMappings[rgbLed1.io3].gpioPin1;

		timingFunction();
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);

		break;
	}


	case 5 : {
		printk(" \n Command of 6 executed , Red and Blue led is on \n");
		printk(" \n Command of 5 executed , Red and Blue led is on \n");
		//5 Blue and Red Led is on
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, 0);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, valueToLed);

		gpioPinToBeOff[0] = pinMappings[rgbLed1.io1].gpioPin1;
		gpioPinToBeOff[1] = 81;
		gpioPinToBeOff[2] = pinMappings[rgbLed1.io3].gpioPin1;

		timingFunction();
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);
		break;
	}
	case 7 : {
		printk(" \n Command of 6 executed , Red GREEN and Blue led is on \n");
		printk(" \n Command of 7 executed , Red, Green and Blue led is on \n");
		//5 Blue Green and Red Led is on
		gpio_set_value_cansleep(pinMappings[rgbLed1.io1].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io2].gpioPin1, valueToLed);
		gpio_set_value_cansleep(pinMappings[rgbLed1.io3].gpioPin1, valueToLed);

		gpioPinToBeOff[0] = pinMappings[rgbLed1.io1].gpioPin1;
		gpioPinToBeOff[1] = pinMappings[rgbLed1.io2].gpioPin1;
		gpioPinToBeOff[2] = pinMappings[rgbLed1.io3].gpioPin1;
		timingFunction();						//Invoking timer
		printk("\n Going to sleep for 0.5 second \n ");
		msleep(500);							//sleeping for 0.5 second for the time period.
		printk("\n Deleting timer \n ");
		hrtimer_cancel(&t1);					//Deleting timer.

		break;
	}
	}
}

/**
 * Timing function.
 */
void timingFunction() {
	unsigned long off_dutycycle = 100ul - (unsigned long)rgbLed1.dutyCycle;
	printk("\n Inside Timing function \n");
	hrtimer_init(&t1, CLOCK_MONOTONIC, HRTIMER_MODE_REL); // Initialising the timer
	t1.function = &call_back_function;					// Setting call back function.
	on_time = ktime_set(0,((unsigned long)rgbLed1.dutyCycle * 200000));	// Setting on time.
	off_time = ktime_set(0,(off_dutycycle * 200000));		//Setting off time.
	hrtimer_start(&t1,on_time,HRTIMER_MODE_REL);			//Starting timer.
}


module_init(rgb_led_driver_init);
module_exit(rgb_led_driver_exit);
MODULE_LICENSE("GPL v2");
