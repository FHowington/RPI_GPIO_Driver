obj-m += cardriver.o

SRC = /home/forbes/PiDriver

PREFIX = arm-linux-gnueabihf-

all:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(PREFIX) -C /home/forbes/linux M=/home/forbes/PiDriver modules

