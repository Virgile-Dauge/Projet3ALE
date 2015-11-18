#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#define ON 0
#define OFF 1
static int irq_number;
static int etat = 0;
static struct gpio mygpios[] = {
		{ 1,GPIOF_OUT_INIT_HIGH, "led1"},
 		{ 2,GPIOF_OUT_INIT_HIGH, "led2"},
		{ 4,GPIOF_OUT_INIT_HIGH, "led3"},
		{ 5,GPIOF_IN, "button"}
};
static void all_off(void){
	gpio_set_value(1,OFF);
	gpio_set_value(2,OFF);
	gpio_set_value(4,OFF); 		
}
static void all_on(void){
	gpio_set_value(1,ON);
	gpio_set_value(2,ON);
	gpio_set_value(4,ON); 		
}
static void set_all(int value){
	gpio_set_value(1,value);
	gpio_set_value(2,value);
	gpio_set_value(4,value); 		
}

void tasklet_button(void){
	etat = 1 - etat;
	set_all(etat);
}
DECLARE_TASKLET(tasklet_button_id, tasklet_button ,0);

irqreturn_t buttonHandler(int irq, void *data){
 	tasklet_schedule(&tasklet_button_id);
	return IRQ_HANDLED;	
}

static int __init tst_init(void)
{
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
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("KHello World!");
