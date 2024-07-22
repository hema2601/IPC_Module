MY_PATH=/home/hema/IPC_Module/module/


obj-m += IPC_module.o


EXTRA_CFLAGS += -O0

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(MY_PATH) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(MY_PATH) clean
