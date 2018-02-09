obj-m += test.o

SRC = /home/forbes/linux/hello

PREFIX = arm-linux-gnueabihf-

all:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(PREFIX) -C /home/forbes/linux M=/home/forbes/linux/hello modules

