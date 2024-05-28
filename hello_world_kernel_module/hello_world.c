#include <linux/module.h>	/* Requerido por todos los módulos */
#include <linux/kernel.h>	/* Definición de KERN_INFO */
MODULE_LICENSE("GPL"); 	/*  Licencia del modulo */
MODULE_DESCRIPTION("Hello world kernel module");
MODULE_AUTHOR("Grupo SdC");


static int __init hellow_init(void) {
    printk(KERN_INFO "Hello, World! From computer systems group!\n");
    return 0;
}

static void __exit hellow_exit(void) {
    printk(KERN_INFO "Goodbye, World! From computer systems group!\n");
}

/* Declaración de funciones init y exit */
module_init(hellow_init);
module_exit(hellow_exit);
