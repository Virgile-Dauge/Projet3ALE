#include <linux/ioctl.h>

/* define struct for RGB color and delay */
struct RGB_delay_data {
	unsigned long delay;
	unsigned char R;
	unsigned char G;
	unsigned char B;
}

#define LED_MAGIC 't'
#define LEDS_OFF _IORW(LED_MAGIC, 0)
#define LEDS_ON _IORW(LED_MAGIC, 1)
#define SET_RGB _IORW(LED_MAGIC, 2, struct RGB_delay_data)
