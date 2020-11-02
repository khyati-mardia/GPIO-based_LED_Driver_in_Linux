#define _POSIX_C_SOURCE 200809L
#define __USE_GNU
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <sstream>
#include <pthread.h>
#include <linux/input.h>
#include <bits/stdc++.h>
#include <signal.h>
#include <string>
#include <errno.h>
#include <ctype.h>
#include "InputSignal.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define MOUSEFILE "/dev/input/event2"
using namespace std;
#define IOC_MAGIC 'k' // defines the magic number
#define SEQ_NR_LOCAL '0' // Sequence number local to the driver.
struct rgbled {
	int dutyCycle;
	int io1;
	int io2;
	int io3;
}; 								// Defined to take file inputs.
#define CONFIG _IOW(IOC_MAGIC, SEQ_NR_LOCAL, struct rgbled) //Defines IOCTL Command.
int fd; 						// File Descriptor
int readFileInputs(string);
void* notifyWhenMouseClickOccurs(void* params);
bool flag = 1;
std::list<InputSignal> inputSignalslist;

//Handles the 'Ctrl+C' 'Ctrl+Z' calls
void handler(int signum){
	close(fd);
	exit(0);
}
/**
 * //We are setting the following integers as values to be sent to the user
	// for glowing of Leds
	// B G R
	// 0 0 0 -----> 0 All Leds are off
	// 0 0 1 -----> 1 Red Led is On
	// 0 1 0 -----> 2 Green Led is On
	// 1 0 0 -----> 4 Blue Led is On
	// 0 1 1 -----> 3 Green and Red Led is on
	// 1 1 0 -----> 6 Blue and Green Led is on
	// 1 0 1 -----> 5 Blue and Red Led is on
	// 1 1 1 -----> 7 All Leds are on.
 */
int main()
{
	signal(SIGTSTP, handler);				  // Handles the 'Ctrl+C, Ctrl+Z' system calls.
	string path;                             //Reading file path to read the input file.
	cout << "Please enter the file path,give a valid complete path inclusive of file name \n";
	cin >> path;                             //Reading the file and populating the pins inputed by the user
	readFileInputs(path);
	pthread_t readingThread;				 //Creating a reading thread that would detect the mouse events
	int s = pthread_create(&readingThread, NULL, notifyWhenMouseClickOccurs,NULL);
	if (s != 0) {
		perror("pthread_create() error");
	}
	std::list<InputSignal>::iterator it = inputSignalslist.begin();
	rgbled rgbFromUser;
	rgbFromUser.dutyCycle = it->durationOfOnPeriod; //Setting the duration of period
	rgbFromUser.io1 = it->redLedOn;					//Setting the red Led pin Value
	rgbFromUser.io2 = it-> greenLed;				// Setting the Green Led pin Value
	rgbFromUser.io3 = it->blueLed;					//Setting the Blue Led Pin.
	cout << "\n Passing the pin data to kernel\n"; 	//Passing pin data from user space to kernelspace
	cout << "\n" << "Duty cycle" << rgbFromUser.dutyCycle << "\n";
	cout << "\n" << "Red Pin" << rgbFromUser.io1 << "\n";
	cout << "\n" << "Green Pin" << rgbFromUser.io2 << "\n";
	cout << "\n" << "Blue Pin" << rgbFromUser.io3 << "\n";
	cout << "Opening file";
	fd = open("/dev/RGBLed", O_RDWR);				//Opening the file.
	if (fd < 0 ){
		printf("Can not open device file.\n");
		exit(0);
	}
	cout << "\n Sending iotcl command to kernel to configure the pins \n";
	int i=ioctl(fd, CONFIG, &rgbFromUser);			//Sending IOCTL Command.
	if ( i < 0) {
		printf("Failed to issue IOCTL command \n");
	}
	cout << "\n Writing integers to glow LEDs \n";
	char userBuffer[10];
	int count = 0;
	int arrayOfStrings[8] = {0,1,2,4,3,6,5,7};
	cout << "\n" << "The command sent by user is" << userBuffer << "\n" ;
	cout << "\n Done writing to buffer , now going to open function \n";
	while(1) {
		sprintf(userBuffer,"%d",arrayOfStrings[count]);		//Writing the integer to be sent to the buffer
		cout << "\n Glowing LED with count \n" << count;
		int l = write(fd, userBuffer, 4);					//Writing to the file from the buffer.
		count++;
		if (count>7 || flag == 0) {							//If mouse click is detected or if count seq is elapsed
			count=0;										// Start over.
			flag = 1;
		}
		if (l < 0){
			cout << l << "\n Value afte writing to device \n " ;
			return errno;
		}
	}
	return 0;
}
/**
 * THis method notifies about the mouse events.
 */
void* notifyWhenMouseClickOccurs(void* dummy) {
	int fd;
	input_event mouseEvent;
	if((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
		perror("opening device");
		exit(EXIT_FAILURE);
	}
	cout << " Mouse File Opened " << "\n";
	while (1) {
		while (read(fd, &mouseEvent, sizeof(struct input_event))) {
			if ((mouseEvent.type == 1 && mouseEvent.code == 272
					&& mouseEvent.value == 1) || (mouseEvent.type == 1 && mouseEvent.code == 273
							&& mouseEvent.value == 1)) {
				//Detecting left mouse click and right mouse click.
				flag =0;
				cout <<" Mouse_Clicked \n";
			};
		}
	}
	return 0;
}

/**
 * THis method is used for reading the file inputs and setting it to
 * an object of type InputSignal.
 */

int readFileInputs(string path) {
	//Iitialising the variables
	ifstream inFile;
	inFile.open(path.c_str());
	std::string line;
	int onDuration;
	int redPin;
	int greenPin;
	int bluePin;
	string str;
	while (inFile.good()) {
		if (!inFile.eof()) {
			if (std::getline(inFile, line)) {
				//Gets the file line by line
				std::istringstream myStreamThing(line);
				myStreamThing >> onDuration;
				InputSignal inputSignal;
				if (onDuration >= 0 && onDuration <= 100) {
					inputSignal.setDurationOfOnPeriod(onDuration);
				}
				else  {
					cout << "This is not a valid Dutycycle \n";
					std::strerror(EINVAL);
					exit(-1);
				}
				myStreamThing >> redPin;
				int variable = redPin;
				if (variable >= 0 && variable <= 13) {
					cout << redPin << " is the Red pin \n" ;
					inputSignal.setRedLedOn(redPin);
				} else {
					cout << "This is not a GPIO pin \n";
					std::strerror(EINVAL);
					exit(-1);

				}
				myStreamThing >> greenPin;
				variable = greenPin;
				if (variable >= 0 && variable <= 13) {
					inputSignal.setGreenLed(greenPin);
					cout << greenPin << " is the green Pin \n";
				} else {
					cout << "This is not a GPIO pin \n";
					std::strerror(EINVAL);
					exit(-1);
				}
				myStreamThing >> bluePin;
				variable = bluePin;
				if (variable >= 0 && variable <= 13) {
					cout << bluePin << " is the Blue Pin \n";
					inputSignal.setBlueLed(bluePin);
				} else {
					cout << "This is not a GPIO Pin \n";
					std::strerror(EINVAL);
					exit(-1);
				}
				inputSignalslist.push_back(inputSignal);
				myStreamThing.clear();
				line.clear();
			}

		}

	}

	inFile.close();
	return (0);
}
