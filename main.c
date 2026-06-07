#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define MY_DEVICE_CLEAR _IO('k', 1)
#define DEVICE_NAME "simpele_char_dev"
#define BUFFER_SIZE 1024

static dev_t  dev_num;
static struct cdev my_cdev;
static struct class *my_class = NULL; 
static struct device *my_device = NULL;    


static char device_buffer[BUFFER_SIZE];
static int buffer_data_size = 0; 


static int dev_open(struct inode *inode, struct file *file){ 
   pr_info("%s: Device opened\n",DEVICE_NAME);
   return 0;
} 

static int dev_release(struct inode *inode, struct file *file){
   pr_info("%s: Device closed\n",DEVICE_NAME);
   return 0;
}

static ssize_t dev_read(struct file *file, char __user *buffer, size_t len, loff_t *offset){
   if (*offset >= buffer_data_size) return 0;
   if (len > buffer_data_size - *offset) len = buffer_data_size - *offset;

   if (copy_to_user(buffer,device_buffer + *offset,len) != 0) return -EFAULT;

   *offset += len;
   pr_info("%s: Read %zu bytes\n", DEVICE_NAME, len);
    return len;   
}






static ssize_t dev_write(struct file *file,const char __user *buffer, size_t len, loff_t *offset){
   if (len > BUFFER_SIZE){
      len = BUFFER_SIZE;
   }
   if (copy_from_user(device_buffer, buffer, len) != 0) {
        return -EFAULT;
   }
   buffer_data_size = len;
   *offset = len;


   pr_info("%s: Written %zu bytes\n", DEVICE_NAME, len);
   return len;
}       


static long dev_ioctl(struct file *file,unsigned int cmd, unsigned long arg){
   switch (cmd) { 
      case MY_DEVICE_CLEAR:
         memset(device_buffer,0,BUFFER_SIZE);
         buffer_data_size = 0;
         pr_info("MY_DEVICE_CLEAR executed.\n");
         return 0;
      default:
         return -EINVAL;
   }            
   return 0;
}
 
static const struct file_operations fops = {
   .owner  = THIS_MODULE,
   .open    = dev_open,
   .release = dev_release,
   .read    = dev_read,    
   .write = dev_write,
   .unlocked_ioctl = dev_ioctl,
}; 


    

static  int __init char_dev_init(void){
   if (alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME)<0){
      return -1;
   }

   cdev_init(&my_cdev,&fops);
   if (cdev_add(&my_cdev,dev_num,1)<0){
      unregister_chrdev_region(dev_num, 1);
        return -1;
   }
     
   my_class = class_create(DEVICE_NAME);
   if (IS_ERR(my_class)){
      cdev_del(&my_cdev);
      unregister_chrdev_region(dev_num,1);
      return PTR_ERR(my_class);
   }

   my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }

    pr_info("%s: Driver registered successfully\n", DEVICE_NAME);
    return 0;
}

static void __exit char_dev_exit(void) {
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("%s: Driver unregistered\n", DEVICE_NAME);
}  
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Ilya");
 MODULE_DESCRIPTION("Hello world module");
 MODULE_VERSION("1.O");


module_init(char_dev_init);
module_exit(char_dev_exit); 
