#include <linux/init.h>
#include <linux/module.h>

/* 内核提供了一个宏来实现模块的参数传递
 * #define module_param(name, type, perm)                          \
 * 	module_param_named(name, name, type, perm)
 *
 * 	name： 表示参数名
 * 	type:  表示参数类型
 * 	perm： 表示参数的读写权限
 *
 *
 *
 * #define MODULE_PARM_DESC(_parm, desc) \
 * 	 __MODULE_INFO(parm, _parm, #_parm ":" desc)
 *
 *
 * /sys/module/mytest/parameters/debug
 * /sys/module/mytest/parameters/mytest
 *
 * insmod mytest.ko mytest=200
 *
 */

static int debug = 1;
module_param(debug,int,0644);
MODULE_PARM_DESC(debug,"enable debugging information");

#define dprintk(args...) \
	if(debug){ \
		printk(KERN_ERR args);\
	}
static int mytest = 100;

module_param(mytest,int,0644);
MODULE_PARM_DESC(mytest,"test for module parameter");
static int my_module_func(void){
	dprintk("my_module_func");
	return 0;
}
static int __init my_test_init(void)
{
	//printk("my first kernel module init...\n");
	dprintk("my first kernel module init...\n");
	dprintk("moduele parameter=%d\n",mytest);
	return 0;
}
static void __exit my_test_exit(void)
{
	printk("Goody");
}
module_init(my_test_init);/*告诉内核模块的入口函数*/
module_exit(my_test_exit);/*告诉内核模块退出的出口函数*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pain Li");
MODULE_DESCRIPTION("my test kernel module");
MODULE_ALIAS("mytest");



/* 
 * EXPORT_SYMBOL()
 * EXPORT_SYMBOL_GPL()
 * 通过cat /proc/kallsyms 查看内核导出的符号表
 */
