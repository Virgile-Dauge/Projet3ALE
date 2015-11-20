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
#include <sonar.h>
#define PIN_SONAR_TRIG 6
#define PIN_SONAR_ECHO 7

//Prototypes
static int device_read(struct file *f, char __user *data, size_t size, loff_t *l);
static int device_write(struct file *f, const char __user *data, size_t size, loff_t *l);
static int device_open(struct inode *i, struct file *f);
static int device_release(struct inode *i, struct file *f);
static long device_ioctl(struct file *f, unsigned int, unsigned long);

void sendStandartTTL(int pin);
static int dist = 0;
static int irq_sonarEcho;
static ktime_t ktime_sonarEchoUp,ktime_sonarEchoResult;

//Liste des pins à reserver
static struct gpio mygpios[] = {
	{ PIN_SONAR_TRIG,GPIOF_OUT_INIT_LOW, "sonarTrig"},
	{ PIN_SONAR_ECHO,GPIOF_IN, "sonarEcho"}
};

int major;
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
	printk(KERN_INFO"read\n");
	return size;
}

static int device_write(struct file *f, const char __user *data, size_t size, loff_t *l)
{
	char inData;
	printk(KERN_INFO"write\n");
	return size;
}
static int device_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO"open\n");
	return 0;
}
static int device_release(struct inode *i, struct file *f)
{
	printk(KERN_INFO"release\n");
	return 0;
}
static long device_ioctl(struct file *f, unsigned int cmd, unsigned long long1)
{
	int retval = 0;
	switch(cmd)
	{
		case GET_DIST :
			
		 break;
		default : retval = -EINVAL; break;
	}
	printk(KERN_INFO"ioctl\n");
	return retval;
}
//Fonction a executer dans une tasklet
void triggerSonar(){
	ktime_sonarEchoUp = ktime_get();
	printk(KERN_INFO "Trigger\n");
	sendStandartTTL(PIN_SONAR_TRIG);
}
void sendStandartTTL(int pin){
	gpio_set_value(pin,1);
	udelay(10);
	gpio_set_value(pin,0);
}
//fonction appellée lors de l'arrivée de l'interuption déclanchée par la pin PIN_SONAR_ECHO
//Il n'est pas possible d'effectuer ce calcul dans une tasklet, étant donné la rapidité du signal à analyser.
irqreturn_t sonarEchoHandler(int irq, void *data){
	//On différencie le cas front montant du cas front decendant
	if(gpio_get_value(PIN_SONAR_ECHO)!=0){
		//lors du front montant, on enregistre le temps
		ktime_sonarEchoUp = ktime_get();
	}else{
		//lors du front descendant, on calcule l'intervale de temps depuis le front montant
		ktime_sonarEchoResult = ktime_sub(ktime_get(),ktime_sonarEchoUp);
		//On calcule la distance en divisant les microsecondes par 58.
		//Le cast permet d'effectuer le calcul sur un système 32bits.
		dist = ((unsigned long)ktime_to_us(ktime_sonarEchoResult))/58;
		printk(KERN_INFO "EEEEEECHOOOOOOO  %d \n",dist);
	}
	return IRQ_HANDLED;
}
static int __init tst_init(void)
{
	int err = 0;
	int status;
	major= register_chrdev(0, "sonar", &fops);
	if(major <0){
		printk(KERN_INFO "Echec de register_chrdev\n");
		status=major;
		return status;
	}
	sonar_class=class_create(THIS_MODULE, "monSonar");
	if(IS_ERR(sonar_class)){
		printk(KERN_INFO "echec class_create\n");
		status=PTR_ERR(sonar_class);
		return status;
	}

	devt = MKDEV(major, 0);
	dev=device_create(sonar_class, NULL, devt, NULL, "sonar");
	status = IS_ERR(dev) ? PTR_ERR(dev):0;

	if(status!=0){
		printk(KERN_ERR "erreur de device_create\n");
		return status;
	}
	//réservation des pins
	err = gpio_request_array(mygpios,ARRAY_SIZE(mygpios));

	//Bind de certains gpio sur des interruptions
	irq_sonarEcho = gpio_to_irq(PIN_SONAR_ECHO);

	//On ajoute des interruptions en précisant le mode de déclanchement
	request_irq(irq_sonarEcho, sonarEchoHandler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,"sonarEchoRISING", NULL);

	printk(KERN_ERR "Tout s'est très bien passé, c'est cool 42128177824\n"); //Fin de l'initialisation
	return err;
}
static void __exit tst_exit(void)
{
	//On libère le tableau des pins réservées
	gpio_free_array(mygpios, ARRAY_SIZE(mygpios));
	//On libére les intérruptions réservées
	free_irq(irq_sonarEcho,NULL);

	device_destroy(sonar_class, devt);
	class_destroy(sonar_class);
	unregister_chrdev(major, "sonar");
	printk(KERN_INFO"SUCccessfuuuly UnLOadED\n");
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("sonar");
