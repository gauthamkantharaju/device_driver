device_driver
=============

Device driver with debugfs support.

- Includes test application to test the device driver

- Replace dmesg() api with pr_() equivalent api


Steps to compile, insert loadable kernal module and create device node

Note: Need to root in order to execute below commands

- make clean;clear;make
- insmod test.ko
- mknod /dev/hello c "major number assigned" 0


Compile test application

- gcc -o testapp testapp.c
