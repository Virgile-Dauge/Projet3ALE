#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/ioctl.h>

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

static void leds_off(void);

static void leds_on(void);

static void set_leds(int value);

static void set_RGB(unsigned char R, unsigned char G, unsigned char B);

static void gradient_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned long delay);

static void set_gradient_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned long delay);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("PinController");

