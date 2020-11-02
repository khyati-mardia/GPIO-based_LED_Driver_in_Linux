/*
 * InputSignal.h
 *
 *  Created on: Sep 29, 2018
 *      Author: chandrika.
 *      The inputs from the user about the dutycycle , IO1, IO2, IO3 are stored in
 *      the form of objects which belong to this class of InputSignal.
 */

#ifndef INPUTSIGNAL_H_
#define INPUTSIGNAL_H_

class InputSignal {
public:
	int durationOfOnPeriod;
	int redLedOn;
	int greenLed;
	int blueLed;

public:
	InputSignal();
	virtual ~InputSignal();

	int getBlueLed() const {
		return blueLed;
	}

	void setBlueLed(int blueLed) {
		this->blueLed = blueLed;
	}

	int getDurationOfOnPeriod() const {
		return durationOfOnPeriod;
	}

	void setDurationOfOnPeriod(int durationOfOnPeriod) {
		this->durationOfOnPeriod = durationOfOnPeriod;
	}

	int getGreenLed() const {
		return greenLed;
	}

	void setGreenLed(int greenLed) {
		this->greenLed = greenLed;
	}

	int getRedLedOn() const {
		return redLedOn;
	}

	void setRedLedOn(int redLedOn) {
		this->redLedOn = redLedOn;
	}



};

#endif /* INPUTSIGNAL_H_ */
