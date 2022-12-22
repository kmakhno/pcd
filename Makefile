obj-m+=main.o

KERN_DIR = /home/w55fa92bsp-2.6.35/linux-2.6.35.4

all:
	make -C $(KERN_DIR) M=$(shell pwd) modules

clean:
	make -C $(KERN_DIR) M=$(shell pwd) clean
