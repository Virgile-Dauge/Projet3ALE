#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "sonar.h"

#define PIN_SONAR_TRIG 6
#define PIN_SONAR_ECHO 7
#define DEBUG 0
//Prototypes
static int device_read(struct file *f, char __user *data, size_t size, loff_t *l);
static int device_write(struct file *f, const char __user *data, size_t size, loff_t *l);
static int device_open(struct inode *i, struct file *f);
static int device_release(struct inode *i, struct file *f);
static long device_ioctl(struct file *f, unsigned int, unsigned long);
int getDist(void);
void triggerSonar(void);

void sendStandartTTL(int pin);

unsigned long timer_interval_ns_All = 0;
static struct hrtimer hr_timerAll;
static ktime_t currtime , interval;


int dist = 0;
static int irq_sonarEcho;
static ktime_t ktime_sonarEchoUp,ktime_sonarEchoResult;

//Liste des pins à reserver
static struct gpio mygpios[] = {
	{ PIN_SONAR_TRIG,GPIOF_OUT_INIT_LOW, "sonarTrig"},
	{ PIN_SONAR_ECHO,GPIOF_IN, "sonarEcho"}
};
struct device *dev;
static struct class *sonar_class;
dev_t devt;

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
		case GET_DIST :
			//retval = getDist();
			//retval = 77;
			copy_to_user(long1,&dist,sizeof(retval));
		break;
		case SET_VAL:
			#if DEBUG
			printk(KERN_INFO "FUCK: %d\n",&long1);
			#endif
		break;
		default : retval = -EINVAL; break;
	}
#if DEBUG
	printk(KERN_INFO"ioctl\n");
#endif
	return retval;
}
//Fonction a executer dans une tasklet
void triggerSonar(){
	ktime_sonarEchoUp = ktime_get();
#if DEBUG
	printk(KERN_INFO "Trigger\n");
#endif
	sendStandartTTL(PIN_SONAR_TRIG);
}
void sendStandartTTL(int pin){
	gpio_set_value(pin,1);
	udelay(10);
	gpio_set_value(pin,0);
} 
/*
int getDist(){
	sonarAck = 0;
	triggerSonar();
	int timeout = 0;
	while(sonarAck != 1){
		msleep(10);
		timeout ++;
		if(timeout>1000){
			return -1;
		}
	}
	return dist;
}
*/
//fonction appellée lors de l'arrivée de l'interuption déclanchée par la pin PIN_SONAR_ECHO
//Il n'est pas possible d'effectuer ce calcul dans une tasklet, étant donné la rapidité du signal à analyser.
irqreturn_t sonarEchoHandler(int irq, void *data){
	//On différencie le cas front montant du cas front decendant
	//udelay(100);
	if(gpio_get_value(PIN_SONAR_ECHO)!=0){
		//printk(KERN_INFO " PAS EEEEEECHOOOOOOO  %d \n",dist);
		//lors du front montant, on enregistre le temps
		ktime_sonarEchoUp = ktime_get();
	}else{
		//printk(KERN_INFO "Avant MAJ  %d \n",dist);
		//lors du front descendant, on calcule l'intervale de temps depuis le front montant
		ktime_sonarEchoResult = ktime_sub(ktime_get(),ktime_sonarEchoUp);
		//On calcule la distance en divisant les microsecondes par 58.
		//Le cast permet d'effectuer le calcul sur un système 32bits.
		dist = ((unsigned long)ktime_to_us(ktime_sonarEchoResult))/58;
		printk(KERN_INFO "EEEEEECHOOOOOOO  %d \n",dist);
	}
	return IRQ_HANDLED;
}
enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	triggerSonar();
  	currtime  = ktime_get();
  	hrtimer_forward(timer_for_restart, currtime , interval);
	return HRTIMER_RESTART;
}

void timer_init(void) {
	interval = ktime_set(1,timer_interval_ns_All);
	hrtimer_init( &hr_timerAll, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
	hr_timerAll.function = &timer_callback;
	hrtimer_start( &hr_timerAll, interval, HRTIMER_MODE_REL );
}
static int __init tst_init(void)
{
	int err = 0;
	int status;
	int retval = 0;
	retval= register_chrdev(MAJOR_NUM, "sonar", &fops);
	if(retval <0){
		#if DEBUG
		printk(KERN_INFO "Echec de register_chrdev\n");
		#endif
		status=retval;
		return status;
	}
	sonar_class=class_create(THIS_MODULE, "monSonar");
	if(IS_ERR(sonar_class)){
		#if DEBUG
		printk(KERN_INFO "echec class_create\n");
		#endif
		status=PTR_ERR(sonar_class);
		return status;
	}

	devt = MKDEV(MAJOR_NUM, 0);
	dev=device_create(sonar_class, NULL, devt, NULL, "sonar"); 
	status = IS_ERR(dev) ? PTR_ERR(dev):0;

	if(status!=0){
		#if DEBUG
		printk(KERN_ERR "erreur de device_create\n");
		#endif
		return status;
	}
	//réservation des pins
	err = gpio_request_array(mygpios,ARRAY_SIZE(mygpios));

	//Bind de certains gpio sur des interruptions
	irq_sonarEcho = gpio_to_irq(PIN_SONAR_ECHO);

	//On ajoute des interruptions en précisant le mode de déclanchement
	request_irq(irq_sonarEcho, sonarEchoHandler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,"sonarEchoRISING", NULL);

	timer_init();

	printk(KERN_ERR "Tout s'est très bien passé, c'est cool 42128177824\n"); //Fin de l'initialisation
	return err;
}
static void __exit tst_exit(void)
{
	//On libère le tableau des pins réservées
	gpio_free_array(mygpios, ARRAY_SIZE(mygpios));
	//On libére les intérruptions réservées
	free_irq(irq_sonarEcho,NULL);
	int ret;
  	ret = hrtimer_cancel( &hr_timerAll );
  	if (ret) printk("The timer was still in use...\n");
  	printk("HR Timer module uninstalling\n");


	device_destroy(sonar_class, devt);
	class_destroy(sonar_class);
	unregister_chrdev(MAJOR_NUM, "sonar");
	printk(KERN_INFO"SUCccessfuuuly UnLOadED\n");
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("sonar");
