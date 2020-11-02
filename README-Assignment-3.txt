Assignment 3 - A GPIO-based LED Driver in Linux

This project is done by Team 8 for A GPIO-based LED Driver in Linux
Before running this project , kindly note the following points 

TEAM 8 
KHYATI MARDIA  1215346587
CHANDRIKA C.S  1215321133


Instructions to run a program on target :

1. Connect pins on Galileo Board according to your file input (Kindly input Pin Numbers between 0-15 except Pin number 7 and 8).

2. File name RGBLed_1.cpp is the program file for User Space and UpdatedModule.c is the program file for Kernel space.
   
3. Kindly set path as Source /opt/iot-devkit/1.7.2/environment-setup-i586-poky-linux and "make all" in the Terminal or Host.
"make all" and "make clean" are the commands for building(compiling) and deleting the built files.

4. Kindly SCP both the object files using following commands.
	i) scp RGBLed_1 root@ip_address_of_target:
	ii) scp UpdatedModule.ko root@ip_address_of_target:

5. To insert the module type , insmod file_name.ko in target (Galileo board) by using following commands.
	insmod UpdatedModule.ko

6. Run the program by using command : ./RGBLed_1

7. Kindly give valid file path inclusive of file name.

8. We are reading inputs from file “input.txt”.
   The file contains one line input with 4 arguments ie (% of Duty Cycle, IO for R , IO for G and  IO for B) which is (50,1,2,3 )   

9.  Mouse is read on Intel Galileo Board by following below path :
	/dev/input/event2

    In order to grant access to reading the mouse event file (evdev) , kindly run the following command in terminal before running the application
	sudo chmod +rw /dev/input/event2

10. Kernel logs can be seen by using "dmesg" command.

11. Make file is modified for X86 architecture by giving relevant compiler paths and also PATH_variable.
 
12. Program is properly commented and warning free.

13. To calculate time  period of each sequence (duration), we know that the period is 0.5 sec, this is for 25 cycles . Therefore each cycle is for 20 ms. The LEDs will blink as per the sequence mentioned in problem statement. The intensity will vary according to the duty cycle specified. The range of duty cycle is of 0 to 100. When a mouse is clicked, the LED sequence will start from the first state. The program has to be terminated using ctrl+c.

14. This code has been tested for PWM values of 0, 10, 50, 100 randomnly , for all the digital gpio pins and has performed as intended. 
