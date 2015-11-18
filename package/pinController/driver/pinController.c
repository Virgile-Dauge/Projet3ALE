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
unsigned long timer_interval_ns = 1e6;
static struct hrtimer hr_timer;
static ktime_t ktime_time, ktime_timeout;
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
/*
static int __init timer_init(void) {
	ktime = ktime_set( 0, timer_interval_ns );
	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timer.function = &timer_callback;
	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
	return 0;
}
*/

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("PinController");
