 #include <linux/module.h>
 #include <linux/kernel.h>

 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Ilya");
 MODULE_DESCRIPTION("Hello world module");




static  int my_init_module(void){
    printk(KERN_INFO "Hello world 1.\n");
    return 0;  
 }    

 static void my_cleanup_module(void){
    printk(KERN_INFO "Bye world 1.\n");
 }  

module_init(my_init_module);
       
module_exit(my_cleanup_module);