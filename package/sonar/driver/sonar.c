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

#define ON 0
#define OFF 1
#define PIN_LED_R 1
#define PIN_LED_G 2
#define PIN_LED_B 4
#define PIN_BUTTON 5
#define PIN_SONAR_TRIG 6
#define PIN_SONAR_ECHO 7

//Prototypes
static int device_read(struct file *f, char __user *data, size_t size, loff_t *l);
static int device_write(struct file *f, const char __user *data, size_t size, loff_t *l);
static int device_open(struct inode *i, struct file *f);
static int device_release(struct inode *i, struct file *f);
static long device_ioctl(struct file *f, unsigned int, unsigned long);

static int irq_button,irq_sonarEcho;
static int etat = 0;
unsigned long timer_interval_ns = 1e6;
static struct hrtimer hr_timer;
static ktime_t ktime_time, ktime_timeout,ktime_sonarEchoUp,ktime_sonarEchoResult;
//Liste des pins à reserver
static struct gpio mygpios[] = {
	{ PIN_SONAR_TRIG,GPIOF_OUT_INIT_LOW, "sonarTrig"},
	{ PIN_SONAR_ECHO,GPIOF_IN, "sonarEcho"}
};

int major;
struct device *dev;
static struct class *ledRGB_class;
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
	gpio_set_value(PIN_LED_R,ON);
	msleep(100);
	gpio_set_value(PIN_LED_R,OFF);
	return size;
}

static int device_write(struct file *f, const char __user *data, size_t size, loff_t *l)
{
	char inData;
	printk(KERN_INFO"write\n");
	copy_from_user(inData,data,1);
	gpio_set_value(PIN_LED_G,inData);
	return size;
}
static int device_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO"open\n");
	gpio_set_value(PIN_LED_B,ON);
	msleep(100);
	gpio_set_value(PIN_LED_B,OFF);
	return 0;
}
static int device_release(struct inode *i, struct file *f)
{
	printk(KERN_INFO"release\n");
	return 0;
}
static long device_ioctl(struct file *f, unsigned int int1, unsigned long long1)
{
	printk(KERN_INFO"ioctl\n");
	return 0;
}
//Fonction a executer dans une tasklet
void triggerSonar(){
	ktime_time = ktime_get();
	ktime_sonarEchoUp = ktime_get();
	printk(KERN_INFO "Trigger\n");
	gpio_set_value(PIN_SONAR_TRIG,1);
	udelay(10);
	gpio_set_value(PIN_SONAR_TRIG,0);
}
void tasklet_sonarEcho(void){
	//printk(KERN_INFO "EEEEEECHOOOOOOO\n");
	if(gpio_get_value(PIN_SONAR_ECHO)!=0){
		printk(KERN_INFO "EEEEEECHOOOOOOO  up\n");
		ktime_sonarEchoUp = ktime_get();
	}else{
		printk(KERN_INFO "EEEEEECHOOOOOOO  down\n");
		ktime_sonarEchoResult = ktime_sub(ktime_get(),ktime_sonarEchoUp);
		int dist = 0;
		dist = ((unsigned long)ktime_to_us(ktime_sonarEchoResult))/58;
		printk(KERN_INFO "EEEEEECHOOOOOOO  %d \n",dist);
	}
}
DECLARE_TASKLET(tasklet_sonarEcho_id, tasklet_sonarEcho ,0);

//fonction appellée lors de l'arrivée de l'interuption
//Dois etre légére
//Ne Dois Pas faire de sleep
irqreturn_t sonarEchoHandler(int irq, void *data){
	if(gpio_get_value(PIN_SONAR_ECHO)!=0){
		ktime_sonarEchoUp = ktime_get();
	}else{
		ktime_sonarEchoResult = ktime_sub(ktime_get(),ktime_sonarEchoUp);
		int dist = 0;
		dist = ((unsigned long)ktime_to_us(ktime_sonarEchoResult))/58;
		printk(KERN_INFO "EEEEEECHOOOOOOO  %d \n",dist);
	}
	//tasklet_schedule(&tasklet_sonarEcho_id);
	return IRQ_HANDLED;
}
static int __init tst_init(void)
{
	ktime_timeout = ktime_set(0,10000);
	ktime_time = ktime_get();
	int err = 0;

	int status;
	major= register_chrdev(0, "ledRGB", &fops);
	if(major <0){
		printk(KERN_INFO "Echec de register_chrdev\n");
		status=major;
		return status;
	}
	ledRGB_class=class_create(THIS_MODULE, "ledRGBAmoi");
	if(IS_ERR(ledRGB_class)){
		printk(KERN_INFO "echec class_create\n");
		status=PTR_ERR(ledRGB_class);
		return status;
	}

	devt = MKDEV(major, 0);
	dev=device_create(ledRGB_class, NULL, devt, NULL, "ledRGB");
	status = IS_ERR(dev) ? PTR_ERR(dev):0;

	if(status!=0){
		printk(KERN_ERR "erreur de device_create\n");
		return status;
	}
	//réservation des pins
	err = gpio_request_array(mygpios,ARRAY_SIZE(mygpios));

	//Bind de certains gpio sur des interruptions
	irq_button = gpio_to_irq(PIN_BUTTON);
	irq_sonarEcho = gpio_to_irq(PIN_SONAR_ECHO);

	//On ajoute des interruptions en précisant le mode de déclanchement
	request_irq(irq_button, buttonHandler, IRQF_TRIGGER_FALLING,"button", NULL);
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

	device_destroy(ledRGB_class, devt);
	class_destroy(ledRGB_class);
	unregister_chrdev(major, "ledRGB");
	printk(KERN_INFO"SUCccessfuuuly UnLOadED\n");
}

module_init(tst_init);
module_exit(tst_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Le trio étrange");
MODULE_DESCRIPTION("sonar");
