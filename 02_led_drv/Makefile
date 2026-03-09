
# 1. 使用不同的开发板内核时, 一定要修改KERN_DIR
# 2. KERN_DIR中的内核要事先配置、编译, 为了能编译内核, 要先设置下列环境变量:
# 3. $(MAKE)引用的是内核目录下的MAKE
# 4. -C表示将工作目录切换到指定的路径，也就是Linux内核目录，读取顶层Makefile
#    内核顶层 Makefile 定义了一个名为 modules 的目标，它会递归进各个子目录寻找要编译的模块
# 5. M表示模块源码目录，即当前目录pwd
#    内核顶层Makefile执行类似：include $(PWD)/Makefile，加载你写的 Makefile
# 6. modules表示当前编译模块
# 7. Kbuild 会看到你定义的obj-m
#    这就告诉它“在这个目录下要生成一个模块 hello_drv.ko，对应的目标对象是 hello_drv.o”
#    obj-m 的作用： 告诉 Kbuild “这个目录下有哪些模块（.ko）要编译”
#KERNEL_DIR = /home/book/100ask_imx6ull-sdk/Linux-4.9.88
KERNEL_DIR := /home/book/my_code/linux_kernel/my-linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
PWD     := $(shell pwd)
TEST_SRC:= $(wildcard *test.c)
TEST_BIN:= $(patsubst %.c,%,$(TEST_SRC))
obj-m	:= led_drv.o

build: kernel_modules drive_test

kernel_modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules 

drive_test: $(TEST_BIN)

%_test: %_test.c
	$(CROSS_COMPILE)gcc -o $@ $<

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules clean
	rm -rf modules.order
	rm -f $(TEST_BIN)

.PHONY: test build kernel_modules drive_test clean
