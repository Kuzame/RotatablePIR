
#ifndef SERVO_H
#define SERVO_H
#define MAX_RIGHT	70 // ideally the Maximum right, even though it can reaches 350~360 safely
#define H_MIDDLE	185
#define V_MID		155
#define MAX_LEFT	316  // ideally the Maximum left, even though it can reaches
#define MAX_UP		60
#define MAX_DOWN	250

#include <avr/io.h>
#include <util/delay.h>

//SHARED VARIABLE
unsigned currentPosition;
unsigned currentVPosition;
unsigned int ServoIsMid() {
	return ((currentPosition==H_MIDDLE)&&(currentVPosition==V_MID));
}
unsigned int ServoIsLeft() {
	return (currentPosition==MAX_LEFT);
}
unsigned int ServoIsRight() {
	return (currentPosition==MAX_RIGHT);
}
unsigned int ServoIsUp() {
	return (currentVPosition==MAX_UP);
}
unsigned int ServoIsDown() {
	return (currentVPosition==MAX_DOWN);
}

void servo_init() {
	//Configure TIMER1
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)

	ICR1=4999;  //fPWM=50Hz (Period = 20ms Standard).

	currentPosition= H_MIDDLE;
	DDRD|=(1<<PD4)|(1<<PD5);   //PWM Pins as Out
}
void initPosition() {
	OCR1A = H_MIDDLE;
	OCR1B = V_MID;
	currentPosition= H_MIDDLE;
	currentVPosition=V_MID;
}

void rotateLeft() {
	OCR1A=MAX_LEFT;
	currentPosition=MAX_LEFT;
}

void rotateMid() {
	OCR1A = H_MIDDLE;
	currentPosition= H_MIDDLE;
}

void rotateRight() {
	OCR1A=MAX_RIGHT;  //left - MAX is 350-360
	currentPosition = MAX_RIGHT;
}

void rotateUp() {
	OCR1B=MAX_UP;
	currentVPosition=MAX_UP;
}

void rotateVMid() {
	OCR1B=V_MID;
	currentVPosition=V_MID;
}

void rotateDown() {
	OCR1B=MAX_DOWN;
	currentVPosition=MAX_DOWN;
}

void rotateRightSlowly(unsigned speed) { // 0 being the slowest
	if(currentPosition>MAX_RIGHT)
	{
		currentPosition-=1+speed; OCR1A = currentPosition;
	}
}

void rotateLeftSlowly(unsigned speed) { // 0 being the slowest
	if(currentPosition<MAX_LEFT)
	{
		currentPosition+=1+speed; OCR1A = currentPosition;
	}
}

void rotateMidSlowly(unsigned speed) { // 0 being the slowest
	if (currentPosition>H_MIDDLE)
	{
		currentPosition-=1+speed;
		OCR1A = currentPosition;
	}
	else
	{
		currentPosition+=1+speed;
		OCR1A = currentPosition;
	}
}

void freeRotation(double sonar) { // value has to be between 0 and 100
	sonar = (-sonar*2.46)+MAX_LEFT;
	OCR1A = sonar;
	currentPosition = sonar;
}

void freeVRotation(double sonar) { // value has to be between 0 and 100
	sonar = (-sonar*1.9)+MAX_DOWN;
	OCR1B = sonar;
	currentPosition = sonar;
}
#endif