obj-m += test.o
test-objs = main.o debugfs.o

ccflags-y += -DDEBUG

make:
	make -C	/lib/modules/${shell uname -r}/build M=${PWD} modules

.PHONY: clean
clean:
	make -C	/lib/modules/${shell uname -r}/build M=${PWD} clean
