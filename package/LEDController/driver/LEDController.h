#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include "LEDCommander.h"

#define ON 0
#define OFF 1
static int etat = 0;
unsigned long timer_interval_ns_All = 10000;
static struct hrtimer hr_timerAll;
static ktime_t currtime , interval;
static unsigned char tick_count=0, color_R, color_G, color_B;
static struct gpio mygpios[] = {
	{ 1,GPIOF_OUT_INIT_HIGH, "led1"},
	{ 2,GPIOF_OUT_INIT_HIGH, "led2"},
	{ 4,GPIOF_OUT_INIT_HIGH, "led3"},
	{ 5,GPIOF_IN, "button"}
};

int retval;
struct device *dev;
static struct class *LED_class;
dev_t devt;

static void leds_off(void);

static void leds_on(void);

static void set_leds(int value);

static void set_RGB(unsigned char R, unsigned char G, unsigned char B);

static void gradient_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned long delay);

static void set_gradient_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned long delay);

struct file_operations fops = {
	.open=device_open,
	.read=device_read,
	.write=device_write,
	.unlocked_ioctl = device_ioctl,
	.release=device_release,
};
static int device_read(struct file *f, char __user *data, size_t size, loff_t *l)
{
	#if DEBUG
	printk(KERN_INFO"read\n");
	#endif
	return size;
}

static int device_write(struct file *f, const char __user *data, size_t size, loff_t *l)
{
	#if DEBUG
	printk(KERN_INFO"write\n");
	#endif
	return size;
}

static int device_open(struct inode *i, struct file *f)
{
	#if DEBUG
	printk(KERN_INFO"open\n");
	#endif
	return 0;
}

static int device_release(struct inode *i, struct file *f)
{
	#if DEBUG
	printk(KERN_INFO"release\n");
	#endif
	return 0;
}

static long device_ioctl(struct file *f, unsigned int cmd, unsigned long long1)
{
	int retval = 0;
	switch(cmd)
	{
		case LEDS_OFF :
			leds_off();
			break;
		case LEDS_ON :
			leds_on();
			break;
		case SET_RGB :
			struct RGB_delay_data RGBdd = &long1;
			set_RGB(RGBdd.R, RGBdd.G, RGBdd.B);
			break;
		case SET_GRADIENT_RGB :
			struct RGB_delay_data RGBdd = &long1;
			set_gradient_RGB(RGBdd.R, RGBdd.G, RGBdd.B, RGBdd.delay);
			break;
		default : retval = -EINVAL; break;
	}
	#if DEBUG
	printk(KERN_INFO"ioctl\n");
	#endif
	return retval;
}


