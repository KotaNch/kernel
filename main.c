#include "linux/cleanup.h"
#include "linux/gfp_types.h"
#include "linux/types.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/cdev.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya");
MODULE_DESCRIPTION("Cyclic Memory kernel Module");

#define BUFFER_SIZE 4096

static char *cyclic_buffer;
static int write_ptr = 0;
static int read_ptr = 0;
static int available_bytes = 0;

static DEFINE_MUTEX(buffer_mutex);

static struct cdev cyclic_cdev;
static dev_t dev_num;




static ssize_t write_data(const char __user *user_buf, size_t count){
   int i;
   mutex_lock(&buffer_mutex);
   for (i = 0; i <count; i++){
      if (available_bytes >= BUFFER_SIZE){
         pr_warn("Cyclic Buffer is full!\n");
         break;
      }
      
      get_user(cyclic_buffer[write_ptr], &user_buf[i]);
      write_ptr = (write_ptr + 1) % BUFFER_SIZE;
      available_bytes++;
   }
   mutex_unlock(&buffer_mutex);
   return i;
}

static ssize_t read_data(char __user *user_buf, size_t count){
   int i;
   mutex_lock(&buffer_mutex);
   
   for (i = 0; i < count; i++){
      if (available_bytes <= 0){
         break;
      }
      
      put_user(cyclic_buffer[read_ptr], &user_buf[i]);
      read_ptr = (read_ptr + 1) % BUFFER_SIZE;
      available_bytes--;
   }
   
   mutex_unlock(&buffer_mutex);
   return i;
}


static ssize_t cyclic_fops_write(struct file *f, const char __user *buf, size_t count, loff_t *offset){
   return write_data(buf,count);
}

static ssize_t cyclic_fops_read(struct file *f, char __user *buf, size_t count, loff_t *offset){
   return read_data(buf,count);
}

static const struct file_operations cyclic_fops = {
   .owner = THIS_MODULE,
   .read = cyclic_fops_read,
   .write = cyclic_fops_write,
};

static int __init cyclic_init(void){
   int ret;
   
   cyclic_buffer = kmalloc(BUFFER_SIZE,GFP_KERNEL);
   if (!cyclic_buffer){
      return -ENOMEM;  
   }
   
   ret = alloc_chrdev_region(&dev_num, 0,1,"cyclic_buf");
   if (ret < 0 ){
      kfree(cyclic_buffer);
      return ret;
   }
   cdev_init(&cyclic_cdev,&cyclic_fops);
   ret = cdev_add(&cyclic_cdev,dev_num,1);
   if (ret < 0){
      unregister_chrdev_region(dev_num,1);
      kfree(cyclic_buffer);
      return ret;
   }
   
   pr_info("Module loaded successfully.\n");
   return 0; 
}



static void __exit cyclic_exit(void){
   cdev_del(&cyclic_cdev);                
   unregister_chrdev_region(dev_num, 1); 
   kfree(cyclic_buffer);
   pr_info("Module unloaded successfully.\n");
}


module_init(cyclic_init);
module_exit(cyclic_exit);