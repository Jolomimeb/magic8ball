#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/prandom.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oritsejolomisan");
MODULE_DESCRIPTION("A kernel module that returns a response to a question posed by the user");
MODULE_VERSION("0.1");

//define the device name
#define DEVICE_NAME "magic8ball"

//define the responses
static const char *responses[] = {
  "It is certain",
  "As I see it, yes",
  "Reply hazy, try again",
  "Don't count on it",
  "It is decidedly so",
  "Most Likely",
  "Ask again later",
  "My reply is no",
  "Without a doubt",
  "Outlook good",
  "Better not to tell you now",
  "My sources say no"
  "Yes,definitely",
  "Yes",
  "Cannot predict now",
  "Outlook not so good",
  "You may rely on it",
  "Signs point to yes",
  "Concentrate and Ask again",
  "Very doubtful"
};

//functions definitions
static int device_driver_open(struct inode *inode, struct file *file);
static int device_driver_close(struct inode *inode, struct file *file);
static ssize_t device_driver_read(struct file *file, char *buff, size_t len, loff_t *offset);
static ssize_t device_driver_write(struct file *file, const char *buff, size_t len, loff_t *offset);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_driver_open,
    .release = device_driver_close,
    .write = device_driver_write,
    .read = device_driver_read,
};

static struct miscdevice magic8ball_driver = {
    .name = DEVICE_NAME,
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &fops,
    .mode = 0444,
};

static int device_driver_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "device open for magic8ball was called\n");
    return 0;
}

static int device_driver_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "device close for magic8ball was called\n");
    return 0;
}

//returns error if try to write
static ssize_t device_driver_write(struct file *filp, const char *buff, size_t length, loff_t *off) {
    return -EPERM;
}

//reads from the device
static ssize_t device_driver_read(struct file *file, char *buff, size_t len, loff_t *offset){
    //check if the offset is greater than 0 to indicat end of file
    if (*offset > 0) {
        return 0; 
    }

    unsigned int random_number;
    get_random_bytes(&random_number, sizeof(random_number));
    random_number %= ARRAY_SIZE(responses);

    size_t len_response = strlen(responses[random_number]);  //size of response

    size_t bytes_to_read;
    if (len > len_response) {
        bytes_to_read = len_response;
    } else {
        bytes_to_read = len;
    }

    if (copy_to_user(buff, responses[random_number], bytes_to_read)) {
        return -EFAULT;
    }

    if (len > len_response) {
        if (put_user('\n', buff + len_response)) {
            return -EFAULT;
        }
        bytes_to_read++; //account for new line
    }
    *offset += len_response; 
    return bytes_to_read;
}

//enter module
static int __init magic8ball_init(void) {
   //register the device name and minor and fops
    int retValue = misc_register(&magic8ball_driver);
    if (retValue < 0) {
        return retValue;
    }
    printk(KERN_INFO "device was loaded.\n");
    return 0;
}

//exit module
static void __exit magic8ball_exit(void) {
   //unregister the device name and minor and fops
    misc_deregister(&magic8ball_driver);
    printk(KERN_INFO "unloading the module\n");
}

module_init(magic8ball_init);
module_exit(magic8ball_exit);
