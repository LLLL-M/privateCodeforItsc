#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>  
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/workqueue.h>

#include "io_ioctl.h"
#include "debug.h"

struct delayed_work io_work;

enum GPIO_PIN_VAL {
	LOW_LEVEL=0,
	HIGH_LEVEL
};

struct global_param {
	spinlock_t lock;
	wait_queue_head_t button_waitq;
    //struct proc_dir_entry *dir;
    //struct proc_dir_entry *entry[128];
};
struct global_param *g_param;

static struct button_irq_desc
{
	int irq;
	int pin;
	int bit;
	char *name;
} button_irqs[] = 
{
	//5ä¸ªæ— çº¿æ¥æ”¶æŒ‰é”®
	{AT91_PIN_PA3, AT91_PIN_PA3, 0, "wireless auto button"},
	{AT91_PIN_PA0, AT91_PIN_PA0, 1, "wireless manual button"},
	{AT91_PIN_PA21, AT91_PIN_PA21, 2, "wireless yellow blink button"},
	{AT91_PIN_PA23, AT91_PIN_PA23, 3, "wireless all red button"},
	{AT91_PIN_PA2, AT91_PIN_PA2, 4, "wireless step button"},
	//5ä¸ªæ‰‹åŠ¨é¢æ¿æŒ‰é”®
	{AT91_PIN_PD14, AT91_PIN_PD14, 0, "auto button"},
	{AT91_PIN_PD15, AT91_PIN_PD15, 1, "manual button"},
	{AT91_PIN_PA29, AT91_PIN_PA29, 2, "yellow blink button"},
	{AT91_PIN_PD16, AT91_PIN_PD16, 3, "all red button"},
	{AT91_PIN_PA28, AT91_PIN_PA28, 4, "step button"},
};
//static volatile int ev_press = 0;
static unsigned char key_data = 0;		//é”®ç›˜æ¿æŒ‰é”®æ•°æ®
static unsigned char wireless_data = 0;	//æ— çº¿æŒ‰é”®æ•°æ®

struct button_irq_desc interrupt_info;  //È«¾Ö£¬¼ÇÂ¼ÖĞ¶ÏgpioĞÅÏ¢
static struct timex txc; //¼ÇÂ¼ÖĞ¶Ï½øÈëÊ±¼ä
static struct rtc_time tm;

static void io_delay_work_func(struct work_struct *work)
{	
	if (at91_get_gpio_value(interrupt_info.pin) == HIGH_LEVEL) {
		rtc_time_to_tm(txc.time.tv_sec,&tm);
		INFO("%s interrupt, time: %d-%d-%d %d:%d:%d\n", interrupt_info.name, tm.tm_year + 1900, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		/*¼ÇÂ¼°´¼üĞÅÏ¢*/
		if (strncmp("wireless", interrupt_info.name, 8) == 0) {
			wireless_data = 1 << interrupt_info.bit;
		} else {
			key_data = 1 << interrupt_info.bit;
		}
	}

	return;
}

static irqreturn_t button_interrupt_handle(int irq, void *dev_id)
{
	struct button_irq_desc *button_irq = (struct button_irq_desc *)dev_id;
	unsigned long flags = 0;

	spin_lock_irqsave(&g_param->lock, flags);

	memcpy(&interrupt_info, button_irq, sizeof(struct button_irq_desc));
	do_gettimeofday(&txc.time);	  /*  ¼ÇÂ¼½øÈëÖĞ¶ÏÊ±¼ä	*/

	spin_unlock_irqrestore(&g_param->lock, flags);

	schedule_delayed_work(&io_work, msecs_to_jiffies(20));

	return IRQ_RETVAL(IRQ_HANDLED);
}

static int is_old_board(void)
{
	return (at91_get_gpio_value(AT91_PIN_PD14) & at91_get_gpio_value(AT91_PIN_PD15)
			& at91_get_gpio_value(AT91_PIN_PD16) & at91_get_gpio_value(AT91_PIN_PA28)
			& at91_get_gpio_value(AT91_PIN_PA29));
}

static int register_interrupt(void)
{
	int ret, i, num = is_old_board() ? 10 : 5;

	for(i = 0; i < num; i++)
	{
		//at91_set_gpio_input(button_irqs[i].pin, 0);
		//(void) gpio_to_irq(button_irqs[i].pin);
		ret = request_irq(button_irqs[i].irq, button_interrupt_handle, IRQ_TYPE_EDGE_RISING, button_irqs[i].name, (void *)&button_irqs[i]);
		if (ret)
		{
			ERR("register %s interrupt fail", button_irqs[i].name);
			i--;
			for(; i >= 0; i--)
			{
				disable_irq(button_irqs[i].irq);
				free_irq(button_irqs[i].irq, (void *)&button_irqs[i]);
			}
			break;
		}
		else
			INFO("register %s interrupt successful!", button_irqs[i].name);
	}
	
	return ret;
}

static void unregister_interrupt(void)
{
	int i, num = is_old_board() ? 10 : 5;
	for (i = 0; i < num; i++)
	{
		if (button_irqs[i].irq < 0)
			continue;
		disable_irq(button_irqs[i].irq);
		free_irq(button_irqs[i].irq, (void *)&button_irqs[i]);
	}
}

static int io_open(struct inode *inode, struct file *filp)
{
	INFO("AT91-SAM9X25 IO-Driver-Open called!!!");
	return 0;
}

static int io_release(struct inode *inode, struct file *file)
{
	//ä¸»æ§ç¨‹åºå…³é—­æ—¶ç†„ç­ç¨‹åºè¿è¡ŒæŒ‡ç¤ºç¯å’ŒGPSæŒ‡ç¤ºç¯
	at91_set_gpio_value(AT91_PIN_PC0, 0);
	at91_set_gpio_value(AT91_PIN_PC2, 0);
	//å…³é—­é”®ç›˜æ¿æŒ‰é”®æŒ‡ç¤ºç¯
	at91_set_gpio_value(AT91_PIN_PD17, 1);
	at91_set_gpio_value(AT91_PIN_PD18, 1);
	at91_set_gpio_value(AT91_PIN_PD19, 1);
	at91_set_gpio_value(AT91_PIN_PD20, 1);
	at91_set_gpio_value(AT91_PIN_PD21, 1);
	INFO("AT91-SAM9X25 IO-Driver-Release called!!!");
	return 0;
}

#if 0
static int irq_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	INFO("read begin\n");
	if (!ev_press)
	{
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else
			wait_event_interruptible(g_param->button_waitq, ev_press == 1);
	}
	ev_press = 0;
	return put_user(key_data, (unchar *)buff);
}
#endif

static uint get_pin(int port)
{
	uint pin = 0;
	
	switch (port) {
		//8ä¸ªè¡ŒäººæŒ‰é”®
		case XRIO_0: pin = AT91_PIN_PA24; break;
		case XRIO_1: pin = AT91_PIN_PA25; break;
		case XRIO_2: pin = AT91_PIN_PA26; break;
		case XRIO_3: pin = AT91_PIN_PA27; break;
		case XRIO_4: pin = AT91_PIN_PC1; break;
		case XRIO_5: pin = AT91_PIN_PC12; break;
		case XRIO_6: pin = AT91_PIN_PC14; break;
		case XRIO_7: pin = AT91_PIN_PC24; break;
		//5ä¸ªæ‰‹åŠ¨é¢æ¿æŒ‰é”®
		case PeriphKey_0: pin = AT91_PIN_PA28; break;
		case PeriphKey_1: pin = AT91_PIN_PA29; break;
		case PeriphKey_2: pin = AT91_PIN_PD14; break;
		case PeriphKey_3: pin = AT91_PIN_PD15; break;
		case PeriphKey_4: pin = AT91_PIN_PD16; break;
		//5ä¸ªæ‰‹åŠ¨é¢æ¿æŒ‰é”®LEDæŒ‡ç¤ºç¯
		case PeriphLED_0: pin = AT91_PIN_PD17; break;
		case PeriphLED_1: pin = AT91_PIN_PD18; break;
		case PeriphLED_2: pin = AT91_PIN_PD19; break;
		case PeriphLED_3: pin = AT91_PIN_PD20; break;
		case PeriphLED_4: pin = AT91_PIN_PD21; break;
		//GPSå’Œç³»ç»Ÿè¿è¡Œçš„LEDæŒ‡ç¤ºç¯
		case LED_0: pin = AT91_PIN_PC0; break;
		case LED_1: pin = AT91_PIN_PC2; break;
		//é»„é—ªæ§åˆ¶
		case YF_CTRL : pin = AT91_PIN_PA18; break;
		//ä¸¤ä¸ªRS485
		case TTYS4: pin = AT91_PIN_PC15; break;
		case TTYS5: pin = AT91_PIN_PC26; break;
	}
	
	return pin;
}

long io_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int pin;
	int port, value;
	int ret = 0;
	
	spin_lock_irq(&g_param->lock);
	switch (cmd) {
		case IO_GET_PIN_STATUS:			
			get_user(port, (int *)arg);
			value = port & 0x1;		//æœ€ä½ä½ä¸ºè®¾ç½®çš„å¼•è„šçŠ¶æ€
			port = port >> 1;	//å…¶ä»–ä½ä¸ºå¼•è„šçš„å®å®šä¹‰
			pin = get_pin(port);
			if (pin == 0) {
				ERR("There is no pin corresponding to the port %d", port);
				ret = -EINVAL;
				break;
			}
			put_user(at91_get_gpio_value(pin), (int *)arg);
			break;
		case IO_SET_PIN_STATUS:
			get_user(port, (int *)arg);
			value = port & 0x1;		//æœ€ä½ä½ä¸ºè®¾ç½®çš„å¼•è„šçŠ¶æ€
			port = port >> 1;	//å…¶ä»–ä½ä¸ºå¼•è„šçš„å®å®šä¹‰
			pin = get_pin(port);
			if (pin == 0) {
				ERR("There is no pin corresponding to the port %d", port);
				ret = -EINVAL;
				break;
			}
			at91_set_gpio_value(pin, value);
			break;
		case IO_GET_BUTTON_STATUS:
#if 0	//å…³é—­æ‰€æœ‰é”®ç›˜æ¿çš„æŒ‰é”®åŠŸèƒ½
			put_user(0, (unchar *)arg);	
#else
			put_user(key_data, (unchar *)arg);
			key_data = 0;
#endif
			break;
		case IO_SET_BUTTON_STATUS:
			get_user(key_data, (unchar *)arg);
			break;
		case IO_GET_WIRELESS_STATUS:
			put_user(wireless_data, (unchar *)arg);
			wireless_data = 0;
			break;
		default: INFO("The cmd isn't supported!"); ret = -EINVAL;
	}
	
	spin_unlock_irq(&g_param->lock);
	return ret;
}

struct file_operations io_ops = 
{
    .owner = THIS_MODULE,
    .release = io_release,
    .open = io_open,
#if 0
	.read = irq_read,
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	.unlocked_ioctl = io_ioctl, 
#else
    .ioctl = io_ioctl,
#endif
};

struct miscdevice misc_dev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gpio",
	.fops = &io_ops,
};

static void io_pin_init(void)
{
	//8¸öĞĞÈË°´¼ü
	at91_set_GPIO_periph(AT91_PIN_PA24,1);
	at91_set_GPIO_periph(AT91_PIN_PA25,1);
	at91_set_GPIO_periph(AT91_PIN_PA26,1);
	at91_set_GPIO_periph(AT91_PIN_PA27,1);
	at91_set_GPIO_periph(AT91_PIN_PC1, 1);
	at91_set_GPIO_periph(AT91_PIN_PC12,1);
	at91_set_GPIO_periph(AT91_PIN_PC14,1);
	at91_set_GPIO_periph(AT91_PIN_PC24,1);
	
	at91_set_gpio_input(AT91_PIN_PA24,0);
	at91_set_gpio_input(AT91_PIN_PA25,0);
	at91_set_gpio_input(AT91_PIN_PA26,0);
	at91_set_gpio_input(AT91_PIN_PA27,0);
	at91_set_gpio_input(AT91_PIN_PC1,0);
	at91_set_gpio_input(AT91_PIN_PC12,0);
	at91_set_gpio_input(AT91_PIN_PC14,0);
	at91_set_gpio_input(AT91_PIN_PC24,0);

	//5¸öÊÖ¶¯Ãæ°å°´¼ü£¬Ê¹ÓÃÏÂÀ­
	at91_set_GPIO_periph(AT91_PIN_PA28,0);
	at91_set_GPIO_periph(AT91_PIN_PA29,0);
	at91_set_GPIO_periph(AT91_PIN_PD14,0);
	at91_set_GPIO_periph(AT91_PIN_PD15,0);
	at91_set_GPIO_periph(AT91_PIN_PD16,0);
	
	at91_set_gpio_input(AT91_PIN_PA28,0);
	at91_set_gpio_input(AT91_PIN_PA29,0);
	at91_set_gpio_input(AT91_PIN_PD14,0);
	at91_set_gpio_input(AT91_PIN_PD15,0);
	at91_set_gpio_input(AT91_PIN_PD16,0);

	at91_set_deglitch(AT91_PIN_PA28,1);
	at91_set_deglitch(AT91_PIN_PA29,1);
	at91_set_deglitch(AT91_PIN_PD14,1);
	at91_set_deglitch(AT91_PIN_PD15,1);
	at91_set_deglitch(AT91_PIN_PD16,1);

	at91_set_pulldown(AT91_PIN_PA28,1);
	at91_set_pulldown(AT91_PIN_PA29,1);
	at91_set_pulldown(AT91_PIN_PD14,1);
	at91_set_pulldown(AT91_PIN_PD15,1);
	at91_set_pulldown(AT91_PIN_PD16,1);
	
	//5¸öÎŞÏß½ÓÊÕ°´¼ü£¬Ê¹ÓÃÏÂÀ­
	at91_set_GPIO_periph(AT91_PIN_PA2,0);
	at91_set_GPIO_periph(AT91_PIN_PA21,0);
	at91_set_GPIO_periph(AT91_PIN_PA3,0);
	at91_set_GPIO_periph(AT91_PIN_PA0,0);
	at91_set_GPIO_periph(AT91_PIN_PA23,0);

	at91_set_gpio_input(AT91_PIN_PA2,0);
	at91_set_gpio_input(AT91_PIN_PA21,0);
	at91_set_gpio_input(AT91_PIN_PA3,0);
	at91_set_gpio_input(AT91_PIN_PA0,0);
	at91_set_gpio_input(AT91_PIN_PA23,0);

	at91_set_deglitch(AT91_PIN_PA2,1);
	at91_set_deglitch(AT91_PIN_PA21,1);
	at91_set_deglitch(AT91_PIN_PA3,1);
	at91_set_deglitch(AT91_PIN_PA0,1);
	at91_set_deglitch(AT91_PIN_PA23,1);

	//ÄÚ²¿ÏÂÀ­
	at91_set_pulldown(AT91_PIN_PA2, 1);
	at91_set_pulldown(AT91_PIN_PA21, 1);
	at91_set_pulldown(AT91_PIN_PA3, 1);
	at91_set_pulldown(AT91_PIN_PA0, 1);
	at91_set_pulldown(AT91_PIN_PA23, 1);
	
	//5¸öÊÖ¶¯Ãæ°å°´¼üLEDÖ¸Ê¾µÆ
	at91_set_GPIO_periph(AT91_PIN_PD17,1);
	at91_set_GPIO_periph(AT91_PIN_PD18,1);
	at91_set_GPIO_periph(AT91_PIN_PD19,1);
	at91_set_GPIO_periph(AT91_PIN_PD20,1);
	at91_set_GPIO_periph(AT91_PIN_PD21,1);
	
	at91_set_gpio_output(AT91_PIN_PD17,1);
	at91_set_gpio_output(AT91_PIN_PD18,1);
	at91_set_gpio_output(AT91_PIN_PD19,1);
	at91_set_gpio_output(AT91_PIN_PD20,1);
	at91_set_gpio_output(AT91_PIN_PD21,1);
	
	//GPSºÍ³ÌĞòÔËĞĞµÄLEDÖ¸Ê¾µÆ
	at91_set_GPIO_periph(AT91_PIN_PC0, 1);
	at91_set_GPIO_periph(AT91_PIN_PC2, 1);
	
	at91_set_gpio_output(AT91_PIN_PC0, 1);
	at91_set_gpio_output(AT91_PIN_PC2, 1);

	//»ÆÉÁ¿ØÖÆ
	at91_set_GPIO_periph(AT91_PIN_PA18,1);
	at91_set_gpio_output(AT91_PIN_PA18,1);
	
	//GPSÊ¹ÄÜ
	at91_set_GPIO_periph(AT91_PIN_PC6,1);
	at91_set_gpio_output(AT91_PIN_PC6,1);

	//Ê¹ÄÜusb
	at91_set_GPIO_periph(AT91_PIN_PC4,1);
	at91_set_gpio_output(AT91_PIN_PC4,1);

	//Ê¹ÄÜÁ½¸öRS485£¬Ä¬ÈÏÎª·¢Ä£Ê½
	at91_set_GPIO_periph(AT91_PIN_PC15,1);
	at91_set_gpio_output(AT91_PIN_PC15,1);
	at91_set_GPIO_periph(AT91_PIN_PC26,1);
	at91_set_gpio_output(AT91_PIN_PC26,1);

	//Ê¹ÄÜ3G/wifiÄ£¿é
	at91_set_GPIO_periph(AT91_PIN_PC7,1);
	at91_set_gpio_output(AT91_PIN_PC7,1);
	at91_set_gpio_output(AT91_PIN_PC7,1);
	at91_set_gpio_output(AT91_PIN_PC7,1);

	//¸´Î»3g/wifiÄ£¿é
	at91_set_GPIO_periph(AT91_PIN_PC3,1);
	at91_set_gpio_output(AT91_PIN_PC3,0);	
	ndelay(110000);
	INFO("WIFI\3G reset!\n");
	at91_set_gpio_output(AT91_PIN_PC3,1);	
	INFO("(WIFI\3G Enable)=%d\n",at91_get_gpio_value(AT91_PIN_PC7));
}

static int __init io_init(void)
{
	int ret = 0;

	g_param = kmalloc(sizeof(*g_param), GFP_KERNEL);
	if (g_param == NULL)
	{
		ERR("kmalloc memory fail");
		return -ENOMEM;
	}

	memset(&interrupt_info, 0, sizeof(struct button_irq_desc));

	io_pin_init();

	spin_lock_init(&g_param->lock);
	init_waitqueue_head(&g_param->button_waitq);
	ret = misc_register(&misc_dev);
	if (ret)
	{
		ERR("misc_register error\n");
		goto out1;
	}

	INIT_DELAYED_WORK(&io_work, io_delay_work_func);

	/*Ò»¶¨Òª·Å×îºó*/
	ret = register_interrupt();
	if (ret)
	{
		goto out2;
	}
	
	INFO("AT91-SAM9X25 insmod device[%s] successful! Compile time: %s, %s", misc_dev.name, __DATE__, __TIME__);
	return 0;
out1:	misc_deregister(&misc_dev);
out2:	kfree(g_param);
	return ret;
}

static void __exit io_exit(void)
{
	if (g_param != NULL)
	{
		misc_deregister(&misc_dev);
		unregister_interrupt();
		kfree(g_param);
		cancel_delayed_work(&io_work);
	}
	INFO("AT91-SAM9X25 rmmod device[%s]! Compile time: %s, %s", misc_dev.name, __DATE__, __TIME__);
}

module_init(io_init);
module_exit(io_exit);

MODULE_AUTHOR("Hikvision Corporation");
MODULE_DESCRIPTION("Signal machine main controller board IO test driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.0.0");
