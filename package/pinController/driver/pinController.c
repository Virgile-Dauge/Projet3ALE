#include "pinController.h"

static void leds_off(void){
	gpio_set_value(1,OFF);
	gpio_set_value(2,OFF);
	gpio_set_value(4,OFF); 		
}
static void leds_on(void){
	gpio_set_value(1,ON);
	gpio_set_value(2,ON);
	gpio_set_value(4,ON); 		
}
static void set_leds(int value){
	gpio_set_value(1,value);
	gpio_set_value(2,value);
	gpio_set_value(4,value); 		
}

static void set_RGB(unsigned char R, unsigned char G, unsigned char B)
{
	color_R = (R+1)/16;
	color_G = (G+1)/16;
	color_B = (B+1)/16;
}

static void gradient_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned long delay)
{
	if (color_R > R) 
		color_R -= 1;
	else if (color_R < R)
		color_R += 1;
	if (color_B > B)
		color_B -= 1;
	else if (color_B < B)
		color_B += 1;
	if (color_G > G)
		color_G -= 1;
	else if (color_G < G)
		color_G += 1;

	if (color_R == R & color_G == G & color_B == B)
		return;

	mdelay(delay);

	gradient_RGB(R, G, B, delay);
}

static void set_gradient_RGB(unsigned char R, unsigned char G, unsigned char B, unsigned long delay)
{
	gradient_RGB((R+1)/16, (G+1)/16, (B+1)/16, delay);
}

enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
  	currtime  = ktime_get();
	
	if (tick_count == 0) {
		if (color_R > 0)
			gpio_set_value(1,ON);
		if (color_G > 0)
			gpio_set_value(2,ON);
		if (color_B > 0)
			gpio_set_value(4,ON);
	} else {
		if (tick_count == color_R)
			gpio_set_value(1,OFF);
		if (tick_count == color_G)
			gpio_set_value(2,OFF);
		if (tick_count == color_B)
			gpio_set_value(4,OFF);
	}
			
	tick_count += 1;
	if (tick_count == 16)
		tick_count = 0;
  	hrtimer_forward(timer_for_restart, currtime , interval);
	return HRTIMER_RESTART;
}

void timer_init(void) {
	hrtimer_init( &hr_timerAll, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timerAll.function = &timer_callback;
	hrtimer_start( &hr_timerAll, interval, HRTIMER_MODE_REL );
}

static int __init tst_init(void)
{
	int err = 0;

	interval = ktime_set(0,timer_interval_ns_All);

	timer_init();

	int i;
	for (i=0; i<10; i++) {
		set_gradient_RGB(255, 0, 0, 100);
		set_gradient_RGB(255, 255, 0, 100);
		set_gradient_RGB(0, 255, 0, 100);
	}
	leds_off();
	set_gradient_RGB(255, 0, 0, 50);
	set_gradient_RGB(0, 255, 0, 50);
	set_gradient_RGB(0, 0, 255, 50);
	set_gradient_RGB(255, 0, 255, 50);

	printk(KERN_INFO"Hello world!\n");
	err = gpio_request_array(mygpios,ARRAY_SIZE(mygpios));
	return err;
}

static void __exit tst_exit(void)
{
	int i;
	for (i=0; i<20; i++) {
		set_gradient_RGB(255, 0, 0, 10);
		mdelay(10);
		set_gradient_RGB(0, 0, 0, 10);
		mdelay(10);
		set_gradient_RGB(255, 255, 255, 10);
		mdelay(10);
	}
	leds_off();

	int ret;
  	ret = hrtimer_cancel( &hr_timerAll );
  	if (ret) printk("The timer was still in use...\n");
  	printk("HR Timer module uninstalling\n");

	printk(KERN_INFO"Goodbye world!\n");
	gpio_free_array(mygpios, ARRAY_SIZE(mygpios));
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("PinController");