#include "asm-generic/errno-base.h"
#include "asm/uaccess.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h> 
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>


#define DEVICE_NAME "cyclic_buffer"
#define BUFFER_SIZE 4096

#define CYCLIC_IOC_MAGIC 'k'
#define CYCLIC_IOC_CLEAR _IO(CYCLIC_IOC_MAGIC,1)
#define CYCLIC_IOC_AVAILABLE _IOR(CYCLIC_IOC_MAGIC,2,int)

static char *cyclic_buffer;
static size_t write_ptr;
static size_t read_ptr;
static size_t available_bytes;


static DEFINE_MUTEX(buffer_mutex);

static struct cdev cyclic_cdev;
static dev_t dev_num;
static struct class *cyclic_class ;
static struct device *cyclic_device ;




static char *cyclic_devnode(const struct device *dev, umode_t *mode){
   if (mode){
      *mode = 0666;
   }
   return NULL;
}




static int dev_open(struct inode *inode, struct file *file){
   pr_info("%s: Device opened\n",DEVICE_NAME);
   return 0;
}

static int dev_release(struct inode *inode, struct file *file){
   pr_info("%s: Device closed\n",DEVICE_NAME);
   return 0;
}


static ssize_t write_data(const char __user *user_buf, size_t count){
   size_t i;
   if (mutex_lock_interruptible(&buffer_mutex)){
      return -ERESTARTSYS;
   }
   for (i = 0; i < count; i++){
      if (available_bytes >= BUFFER_SIZE)
         break;
      if (get_user(cyclic_buffer[write_ptr], &user_buf[i])){
         mutex_unlock(&buffer_mutex);
         return i ? (size_t)i : -EFAULT;
      }

      write_ptr = (write_ptr + 1) % BUFFER_SIZE;
      available_bytes++;
   }

   mutex_unlock(&buffer_mutex);
   return (ssize_t)i;
}


static ssize_t read_data(char __user *user_buf, size_t count){
   size_t i;
   
   if (mutex_lock_interruptible(&buffer_mutex)){
      return -ERESTARTSYS;
   }

   for (i = 0; i < count; i++){
      if (available_bytes == 0){
         break;
      }
      if (put_user(cyclic_buffer[read_ptr], &user_buf[i])) {
         mutex_unlock(&buffer_mutex);
         return i ? (ssize_t)i : -EFAULT;
      }
      read_ptr = (read_ptr + 1) % BUFFER_SIZE;
      available_bytes--;
      
   } 

   
   mutex_unlock(&buffer_mutex);
   return (ssize_t)i;
}

static void clear_buffer(void){
   memset(cyclic_buffer,0,BUFFER_SIZE);
   write_ptr = 0;
   read_ptr = 0;
   available_bytes = 0;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
   int avail;


   switch (cmd){
      case CYCLIC_IOC_CLEAR:
         if(mutex_lock_interruptible(&buffer_mutex)){
            return -ERESTARTSYS;
         }
         clear_buffer();
         mutex_unlock(&buffer_mutex);
         pr_info("%s: buffer cleaned\n", DEVICE_NAME);
         return 0;
      case CYCLIC_IOC_AVAILABLE:
         if (mutex_lock_interruptible(&buffer_mutex)){
            return -ERESTARTSYS;
         }
         avail = (int)available_bytes;
         mutex_unlock(&buffer_mutex);

         if (put_user(avail, (int __user *)arg)){
            return -EFAULT;
         }
      default:
         return -ENOTTY;
   }
}

static ssize_t cyclic_fops_write(struct file *f, const char __user *buf, size_t count, loff_t *offset){
   return write_data(buf,count);
}

static ssize_t cyclic_fops_read(struct file *f, char __user *buf, size_t count, loff_t *offset){
   return read_data(buf,count);
}

static const struct file_operations cyclic_fops = {
   .owner = THIS_MODULE,
   .open = dev_open,
   .release = dev_release,
   .read = cyclic_fops_read,
   .write = cyclic_fops_write,
   .llseek = noop_llseek,
   .unlocked_ioctl =dev_ioctl,
};

static int __init cyclic_init(void){
   int ret;

   cyclic_buffer = kzalloc(BUFFER_SIZE,GFP_KERNEL);
   if (!cyclic_buffer){
      return -ENOMEM;  
   }
   
   ret = alloc_chrdev_region(&dev_num, 0,1, DEVICE_NAME);
   if (ret < 0 ){
      goto err_free_buffer;
   }
   
   cdev_init(&cyclic_cdev,&cyclic_fops);
   ret = cdev_add(&cyclic_cdev,dev_num,1);
   if (ret < 0){
      goto err_unregister_region;
   }
   cyclic_class = class_create(DEVICE_NAME);
   if (IS_ERR(cyclic_class)){
   }
      ret = PTR_ERR(cyclic_class);
      goto err_del_cdev;
   cyclic_class->devnode = cyclic_devnode;
  
   cyclic_device = device_create(cyclic_class,NULL,dev_num,NULL,DEVICE_NAME);
   if (IS_ERR(cyclic_device)){
      ret = PTR_ERR(cyclic_device);
      goto err_destroy_class;
   }
   
   pr_info("%s: loaded, major=%d minor=%d\n", DEVICE_NAME,MAJOR(dev_num), MINOR(dev_num));

   return 0; 
err_destroy_class:
   class_destroy(cyclic_class);

err_del_cdev:
   cdev_del(&cyclic_cdev);
err_unregister_region:
   unregister_chrdev_region(dev_num, 1);
err_free_buffer:
   kfree(cyclic_buffer);
   return ret;
}




static void __exit cyclic_exit(void){
   device_destroy(cyclic_class, dev_num);
   class_destroy(cyclic_class);
   cdev_del(&cyclic_cdev);                
   unregister_chrdev_region(dev_num, 1); 
   kfree(cyclic_buffer);
   pr_info("Module unloaded successfully.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya");
MODULE_DESCRIPTION("Simple character device with cyclic buffer");

module_init(cyclic_init);
module_exit(cyclic_exit);