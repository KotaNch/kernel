#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define BUFFER_SIZE 128

static char kernel_buffer[BUFFER_SIZE];


static int my_int = 10;
module_param(my_int,int,S_IRUGO);

static char *name = "World";
module_param(name,charp,0644);
MODULE_PARM_DESC(name,"Name to greet");

static int hello_proc_show(struct seq_file *m,void *v){
   seq_printf(m,"%s\n", kernel_buffer);
   return 0;
} 

static int hello_proc_open(struct inode *inode, struct file *file){
   return single_open(file, hello_proc_show,NULL);
} 

static ssize_t hello_proc_write(struct file *file,const char __user *buffer, size_t len, loff_t *offset){
   if (len > BUFFER_SIZE-1){
       return -EINVAL;
   }
   if (copy_from_user(kernel_buffer, buffer, len)) {
        return -EFAULT;
   }
   kernel_buffer[len] = '\0';

   printk(KERN_INFO "Received from user: %s\n", kernel_buffer);

   return len;
}

static const struct proc_ops hello_proc_fops = {
    .proc_open    = hello_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write = hello_proc_write,
};


static  int __init hello_proc_init(void){
   proc_create("hello_proc", 0666, NULL, &hello_proc_fops);
   // printk(KERN_INFO "Hello %s \n",name);
   return 0;  
}    

static void __exit my_cleanup_module(void){
   remove_proc_entry("hello_proc",NULL);
   // printk(KERN_INFO "Bye world 1.\n");
 }  
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Ilya");
 MODULE_DESCRIPTION("Hello world module");


module_init(hello_proc_init);
module_exit(my_cleanup_module); 
