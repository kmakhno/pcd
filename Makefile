obj-m+=main.o
HOST_KERN_DIR = /usr/src/linux-headers-$(shell uname -r) 
KERN_DIR = /home/w55fa92bsp-2.6.35/linux-2.6.35.4

all:
	make -C $(HOST_KERN_DIR) M=$(shell pwd) modules

clean:
	make -C $(HOST_KERN_DIR) M=$(shell pwd) clean
