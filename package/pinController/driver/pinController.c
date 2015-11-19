#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>

#define ON 0
#define OFF 1
static int irq_number;
static int etat = 0;
unsigned long timer_interval_ns_All = 10000;
static struct hrtimer hr_timerAll;
static ktime_t ktime_time, ktime_timeout, ktimeAll;
static unsigned char tick_count=0, color_R, color_G, color_B;
static struct gpio mygpios[] = {

	{ 1,GPIOF_OUT_INIT_HIGH, "led1"},
	{ 2,GPIOF_OUT_INIT_HIGH, "led2"},
	{ 4,GPIOF_OUT_INIT_HIGH, "led3"},
	{ 5,GPIOF_IN, "button"}
};
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
//Fonction a executer dans une tasklet
void tasklet_button(void){
	if (ktime_compare(ktime_sub(ktime_get(),ktime_time), ktime_timeout) == 1) {
		etat = 1 - etat;
		set_leds(etat);
		ktime_time = ktime_get();
	}
}
DECLARE_TASKLET(tasklet_button_id, tasklet_button ,0);

//fonction appellée lors de l'arrivée de l'interuption
//Dois etre légére
//Ne Dois Pas faire de sleep
irqreturn_t buttonHandler(int irq, void *data){
	tasklet_schedule(&tasklet_button_id);
	return IRQ_HANDLED;	
}

static int __init tst_init(void)
{
	ktime_timeout = ktime_set(0,10000);
	ktime_time = ktime_get();
	int err = 0, 
	pause = 1, 
	somme_pause=0, 
	temps_total=2000;

	set_RGB(20, 45, 86);
	timer_init();

	printk(KERN_INFO"Hello world!\n");
	err = gpio_request_array(mygpios,ARRAY_SIZE(mygpios));
	irq_number = gpio_to_irq(5);
	request_irq(irq_number, buttonHandler, IRQF_TRIGGER_FALLING,"button", NULL);
	return err;
}
static void __exit tst_exit(void)
{
	printk(KERN_INFO"Goodbye world!\n");
	gpio_free_array(mygpios, ARRAY_SIZE(mygpios));
	free_irq(irq_number,NULL);
}

static void set_RGB(unsigned char R, unsigned char G, unsigned char B)
{
	color_R = R;
	color_G = G;
	color_B = B;
}

enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
  	ktime_t currtime , interval;
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
	
	interval = ktime_set(0,timer_interval_ns_All);
			
	tick_count += 1;
  	hrtimer_forward(timer_for_restart, currtime , interval);
	return HRTIMER_RESTART;
}

static int timer_init(void) {
	ktime = ktime_set( 0, timer_interval_ns );
	hrtimer_init( &hr_timerAll, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timer.function = &timer_callback;
	hrtimer_start( &hr_timerAll, ktimeAll, HRTIMER_MODE_REL );
	return 0;
}

static void timer_exit(void) {
	int ret;
  	ret = hrtimer_cancel( &hr_timer );
  	if (ret) printk("The timer was still in use...\n");
  	printk("HR Timer module uninstalling\n");
	
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("PinController");
