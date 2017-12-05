/*
 * KeypadLCD.c
 *
 * Created: 6/3/2017 3:44:09 PM
 *  Author: AdrianH
 */ 

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <avr/io.h>
#include "io.h"
#include "bit.h"
#include "timer.h"
#include "Servo.h"
// KEYPAD----------------------------------------------------------------------

unsigned char recentInput = 'C';
unsigned char GetKeypadKey() {

	PORTA = 0xEF; // Enable col 4 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('1'); }
	if (GetBit(PINA,1)==0) { return('4'); }
	if (GetBit(PINA,2)==0) { return('7'); }
	if (GetBit(PINA,3)==0) { return('*'); }

	// Check keys in col 2
	PORTA = 0xDF; // Enable col 5 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('2'); }
	if (GetBit(PINA,1)==0) { return('5'); }
	if (GetBit(PINA,2)==0) { return('8'); }
	if (GetBit(PINA,3)==0) { return('0'); }
	// ... *****FINISH*****

	// Check keys in col 3
	PORTA = 0xBF; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { return('3'); }
	if (GetBit(PINA,1)==0) { return('6'); }
	if (GetBit(PINA,2)==0) { return('9'); }
	if (GetBit(PINA,3)==0) { return('#'); }
	// ... *****FINISH*****

	// Check keys in col 4
	PORTA = 0x7F; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINA,0)==0) { recentInput ='A'; return('A'); }
	if (GetBit(PINA,1)==0) { recentInput ='B'; return('B'); }
	if (GetBit(PINA,2)==0) { recentInput ='C'; return('C'); }
	if (GetBit(PINA,3)==0) { recentInput ='D'; return('D'); }

	return(recentInput); // default value

}

//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
b = c;
	}
	return 0;
}
//--------End find GCD function ----------------------------------------------

//--------Task scheduler data structure---------------------------------------
// Struct for Tasks represent a running process in our simple real-time operating system.
typedef struct _task {
	/*Tasks should have members that include: state, period,
		a measurement of elapsed time, and a function pointer.*/
	signed char state; //Task's current state
	unsigned long int period; //Task period
	unsigned long int elapsedTime; //Time elapsed since last task tick
	int (*TickFct)(int); //Task tick function
} task;

//--------End Task scheduler data structure-----------------------------------

//--------Shared Variables----------------------------------------------------
unsigned char pause = 0;

//For LCD
unsigned int cursorPosition = 1;
unsigned int cooldown = 20;
unsigned howManyTimesTriggered;
unsigned userTimerInput=0;
unsigned userIntervalInput=0;

//For Keypad
unsigned char input;
unsigned char currentInput = 'C';
unsigned appendNumber=0;

//For Alarm
unsigned songBit = 0;
#define c3 261.63/2 
#define d3 293.66/2 
#define e3 329.63/2 
#define f3 349.23/2
#define fs3 370/2
#define g3 392/2
#define a3 440/2
#define b3 493.88/2
#define c4 261.63
#define d4 293.66
#define e4 329.63
#define f4 349.23
#define fs4 369.99
#define g4 392
#define a4 440
#define b4 493.88
#define c5 523.25
#define d5 293.66*2
#define e5 329.63*2
#define f5 349.23*2
#define g5 392*2
#define a5 440*2
#define b5 493.88*2
#define nu 0 // most/major notes from c3 to b5

double song [] = {a4,c4,d5,e5,	a5,a5,nu,nu,	a4,c4,d5,e5,	a5,a5,nu,nu,	a4,c4,d5,e5,	a5,a5,nu,nu,	a4,c4,d5,e5,	a5,a5,nu,nu};
//double song [] = {a5,g5,e5,d5,	g5,e5,d5,c5,	e5,d5,c5,a4,	d5,c5,a4,g4,	c5,a4,g4,e4,	a4,g4,e4,d4,	g4,e4,c4,c4,	a3,a3,a3,a3};

//For ULTRASONAR SENSOR
unsigned char dist[5];
unsigned int numbers=0;
//--------End Shared Variables------------------------------------------------

//--------User defined FSMs---------------------------------------------------
//Enumeration of states.
enum changingState {nothing, something, nothingToSomething, somethingToNothing} cState = nothing;
enum songIsPlayingState {songOff, songOn} songIsPlaying = songOff;
enum SM1_States { SM1_Idle,	SM1_A, SM1_B, SM1_B_start, SM1_C, SM1_D} KeypadState= SM1_C;
enum SM1_States2 { SM1_Idle2, SM1_1, SM1_2, SM1_3, SM1_4, SM1_5, SM1_6, SM1_7, SM1_8, SM1_9, SM1_0, SM1_star, SM1_pound } KeypadInput;
enum SM1_States3 { differentState, sameState} KeypadChangingState=differentState;
enum SM1_States4 { differentInput, sameInput} KeyPadChangingInput = differentInput;
enum SM2_States { startDetection, pauseDetection} PIR_State=pauseDetection;
enum SM5_States { moveLeft, moveRight, moveMid, moveMid2} servoState = moveRight;
enum SM6_States { firstTime, notFirstTime } LCD_State = firstTime;

enum BStates {init, askTimer, askTimerWaitInput, askInterval, askIntervalWaitInput, beginCountdown, BPause, ended, ended2} bStates = init;

// 1 Keypad
// 2 PIR
// 3 Alarm
// 4 Distance sensor
// 5 Servomotor
// 6 LCD

int SMTick1(int state) {
	input = GetKeypadKey();
	switch (input) {
		//Major state transition
		case 'A': if(KeypadState==SM1_A) KeypadChangingState=sameState; else {KeypadChangingState=differentState; KeypadState =  SM1_A;} break;
		case 'B': if(KeypadState==SM1_B) KeypadChangingState=sameState; else {KeypadChangingState=differentState; KeypadState =  SM1_B;} break;
		case 'C': if(KeypadState==SM1_C) KeypadChangingState=sameState; else {KeypadChangingState=differentState; KeypadState =  SM1_C;} break;
		case 'D': if(KeypadState==SM1_D) KeypadChangingState=sameState; else {KeypadChangingState=differentState; KeypadState =  SM1_D;} break;
		
		//(minor) Input
		//case '\0': //KeypadState = SM1_Idle;
		//			KeypadInput=SM1_Idle2; break;
		case '1': if(KeypadInput==SM1_1) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_1;} break; // hex equivalent
		case '2': if(KeypadInput==SM1_2) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_2;} break;
		case '3': if(KeypadInput==SM1_3) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_3;} break;
		case '4': if(KeypadInput==SM1_4) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_4;} break;
		case '5': if(KeypadInput==SM1_5) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_5;}  break;
		case '6': if(KeypadInput==SM1_6) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_6;}  break;
		case '7': if(KeypadInput==SM1_7) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_7;}  break;
		case '8': if(KeypadInput==SM1_8) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_8;} break;
		case '9': if(KeypadInput==SM1_9) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_9;} break;
		case '*': if(KeypadInput==SM1_star) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_star;} break;
		case '0': if(KeypadInput==SM1_0) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_0;} break;
		case '#': if(KeypadInput==SM1_pound) KeyPadChangingInput=sameInput; else {KeyPadChangingInput=differentInput; KeypadInput = SM1_pound;} break;
		default: break; // Should never occur. Middle LED off.
	}
	//transitions

	//actions
	if (input!=currentInput) 
		switch(KeypadState){
			case SM1_B:
				switch(bStates) {
					case askTimerWaitInput:
						switch(input) {
							case '1': userTimerInput*=10; userTimerInput+=1; LCD_WriteString(currentPosition++, "1");break;
							case '2': userTimerInput*=10; userTimerInput+=2; LCD_WriteString(currentPosition++, "2");break;
							case '3': userTimerInput*=10; userTimerInput+=3; LCD_WriteString(currentPosition++, "3");break;
							case '4': userTimerInput*=10; userTimerInput+=4; LCD_WriteString(currentPosition++, "4");break;
							case '5': userTimerInput*=10; userTimerInput+=5; LCD_WriteString(currentPosition++, "5");break;
							case '6': userTimerInput*=10; userTimerInput+=6; LCD_WriteString(currentPosition++, "6");break;
							case '7': userTimerInput*=10; userTimerInput+=7; LCD_WriteString(currentPosition++, "7");break;
							case '8': userTimerInput*=10; userTimerInput+=8; LCD_WriteString(currentPosition++, "8");break;
							case '9': userTimerInput*=10; userTimerInput+=9; LCD_WriteString(currentPosition++, "9");break;
							case '0': userTimerInput*=10; userTimerInput+=0; LCD_WriteString(currentPosition++, "0");break;
							case '#': bStates= askInterval; break;
							default: break; // Should never occur. 
						}
						break;
					case askIntervalWaitInput:
						switch(input) {
							case '1': userIntervalInput*=10; userIntervalInput+=1; LCD_WriteString(currentPosition++, "1");break;
							case '2': userIntervalInput*=10; userIntervalInput+=2; LCD_WriteString(currentPosition++, "2");break;
							case '3': userIntervalInput*=10; userIntervalInput+=3; LCD_WriteString(currentPosition++, "3");break;
							case '4': userIntervalInput*=10; userIntervalInput+=4; LCD_WriteString(currentPosition++, "4");break;
							case '5': userIntervalInput*=10; userIntervalInput+=5; LCD_WriteString(currentPosition++, "5");break;
							case '6': userIntervalInput*=10; userIntervalInput+=6; LCD_WriteString(currentPosition++, "6");break;
							case '7': userIntervalInput*=10; userIntervalInput+=7; LCD_WriteString(currentPosition++, "7");break;
							case '8': userIntervalInput*=10; userIntervalInput+=8; LCD_WriteString(currentPosition++, "8");break;
							case '9': userIntervalInput*=10; userIntervalInput+=9; LCD_WriteString(currentPosition++, "9");break;
							case '0': userIntervalInput*=10; userIntervalInput+=0; LCD_WriteString(currentPosition++, "0");break;
							case '#': bStates=beginCountdown;  PIR_State = startDetection;
							LCD_ClearScreen(); _delay_ms(2); LCD_Cursor(1);LCD_WriteData(0x03);LCD_WriteString(2, " Playing DotA ");LCD_WriteData(0x03);
							LCD_Cursor(32); LCD_WriteData(0x07);break;
							default: break; // Should never occur.
						}
						break;
					default: break;
					
				}
				break;
			default: break;
		}
		
		currentInput=input;
	return state;
}

// PIR sensor
int SMTick2(int state) {
	switch(PIR_State) {
		case startDetection:
			if(KeypadState==SM1_A || bStates == beginCountdown) {
				switch(cState) {
					case nothing: break;
					case something: break;
					case somethingToNothing: break;
					case nothingToSomething: break;
					default: break;
				}
				
				if (!(PINB&0x08)) {
					if(cState==something) cState= somethingToNothing;
					else cState = nothing;
				}
				else {
					if(cState==nothing) {
						cState=nothingToSomething;howManyTimesTriggered++;
					}
					else cState = something;
				}
			}
			break;
		
		case pauseDetection: break;
		default: break;
	}
	return state;
}

// Alarm
int SMTick3(int state) {
	
	if (cState==nothingToSomething||songIsPlaying==songOn) {
		songIsPlaying= songOn;
	}
	//actions
	switch(songIsPlaying){
		case songOff: break;
		case songOn: {
				set_PWM(song[songBit++]);
				if(songBit>=16) {
					songBit=0; songIsPlaying = songOff; set_PWM(0);
				}
			}break;
		default: break;
	}
	return state;
}

// Ultrasonic/Distance Sensor
int SMTick4(int state) {
	
	if(KeypadState==SM1_D) {
		numbers = (PIND&0x0F) | ((PIND&0xC0)>>2); // updating distance VALUE
		numbers-=5;
		if (numbers<51)	itoa(numbers, dist, 10);
		else itoa(50, dist, 10);
	}
	return state;
}

// Servo motor
int SMTick5(int state) {
	//transitions
	
	switch(KeypadState) {
		case SM1_B:
			if(bStates==BPause && cooldown==1) {
			 	switch(servoState) {
				 	case moveMid: servoState=moveLeft;break;
				 	case moveMid2: servoState=moveRight;break;
			 		case moveLeft: servoState = moveMid2; break;
			 		case moveRight: servoState = moveMid;break;
			 		default: servoState = moveLeft; break;
			 	}
				switch(servoState) {
					case moveMid: rotateMid(); break;
					case moveMid2: rotateMid(); break;
					case moveLeft: rotateLeft(); break;
					case moveRight: rotateRight(); break;
					default: break;
				}	
			}
			break;
		case SM1_C:
			switch(KeypadInput) {
				case SM1_2: if(!ServoIsUp()) rotateUp(); LCD_WriteString(17, "Moving up   "); break;
				case SM1_4: if(!ServoIsLeft()) rotateLeft(); LCD_WriteString(17, "Moving left "); break;
				case SM1_5: if(!ServoIsMid()) initPosition(); LCD_WriteString(17, "Moving mid  "); break;
				case SM1_6: if(!ServoIsRight()) rotateRight(); LCD_WriteString(17, "Moving right"); break;
				case SM1_8: if(!ServoIsDown()) rotateDown(); LCD_WriteString(17, "Moving down "); break;
				case SM1_star: break;
				case SM1_pound: break;
				default: break;
			}
		
			break;
		case SM1_D:
			switch(KeypadInput) {
				case SM1_star:
					if (numbers>=0 && numbers <51) freeRotation((numbers)*2); break;
				case SM1_pound:
					if (numbers>=0 && numbers <51) freeVRotation((numbers)*2); break;
				default: break;
			}
			break;
		default: break;
	}
	
// 	if(isLeft()) 
// 		servoState = moveRight;
// 	else if(isRight())
// 		servoState = moveLeft;
// 	//actions


// 		switch(cState){
// 			case somethingToNothing:
// 			rotateLeft(); break;
// 			case nothingToSomething:
// 			rotateMid(); break;
// 			case nothing: rotateRight(); break;
// 			case something: LCD_DisplayString(1, "Something"); break;
// 			default: break;
// 			
// 		}

	return state;
}

// LCD Display
int SMTick6(int state) {
	
	//actions
	switch(KeypadState) {
		//---------------------------------------------- A -----------------------------------------------------
		case SM1_A:
			if(KeypadChangingState == differentState) {
				LCD_ClearScreen();
				_delay_ms(2);
				LCD_WriteString(1, "Starting in 2s");
				cooldown=0;
				PIR_State = pauseDetection;bStates=askTimer;
			}
			if (cooldown==10) LCD_WriteString(1, "Starting in 1s");
			if(cooldown++==20) {
				LCD_ClearScreen();
				_delay_ms(2);
				LCD_Cursor(1);
				LCD_WriteData(0x03);
				LCD_WriteString(2, " Playing DotA ");
				LCD_WriteData(0x03);
				LCD_WriteString(17, "In-game "); LCD_Cursor(32); LCD_WriteData(0x07);
				PIR_State = startDetection;
			}
			else if(cooldown>20)
			switch(cState){
				case somethingToNothing:
				LCD_ClearScreen();
				_delay_ms(2);
				LCD_Cursor(1);
				LCD_WriteData(0x03);
				LCD_WriteString(2, " Playing DotA ");
				LCD_WriteData(0x03); 
				LCD_WriteString(17, "In-game "); LCD_Cursor(32); LCD_WriteData(0x07);break;
				case nothingToSomething:LCD_DisplayString(2, " Hello THERE  "); LCD_WriteData(0x04);
				LCD_Cursor(1);LCD_WriteData(0x04);
				LCD_WriteString(17, "Pause game F9"); LCD_Cursor(32); LCD_WriteData(0x06); break;
// 				case nothing: LCD_DisplayString(1, "Nothing"); break;
// 				case something: LCD_DisplayString(1, "Something"); break;
				default: break;
			}
			break;
		//---------------------------------------------- B -----------------------------------------------------
		case SM1_B:
			switch(bStates) {
				case init: bStates=askTimer; break;
				case askTimer: LCD_DisplayString(1, "Time(s): "); bStates=askTimerWaitInput; PIR_State = pauseDetection; currentPosition=17; userTimerInput=0; break;
				case askInterval: LCD_DisplayString(1, "RotationInterval"); bStates=askIntervalWaitInput; howManyTimesTriggered=0; cooldown=0; currentPosition=17; userIntervalInput=0; break;
				case beginCountdown: 
					switch(cState){
						case somethingToNothing:
							LCD_ClearScreen(); _delay_ms(2); LCD_Cursor(1);LCD_WriteData(0x03);LCD_WriteString(2, " Playing DotA ");LCD_WriteData(0x03);
							LCD_Cursor(32);LCD_WriteData(0x07); break;
						case nothingToSomething:LCD_DisplayString(2, " Hello THERE  "); LCD_WriteData(0x04);LCD_Cursor(1);LCD_WriteData(0x04);
							LCD_Cursor(32);LCD_WriteData(0x07); 
							//LCD_Cursor(32); LCD_WriteData(0x06);
							break;
						default: break;
					}
					if ((++cooldown%10)==0) {
						itoa(--userTimerInput, dist,10);
						LCD_WriteString(25, dist);
						if (userTimerInput<10) LCD_WriteString(26, " ");
					}
					if(cooldown==userIntervalInput*10) {
						bStates=BPause; PIR_State=pauseDetection;
						cooldown=0;LCD_Cursor(32); LCD_WriteData(0x06);
					}
					if (userTimerInput==0) bStates=ended;
				
					break;
					
				case BPause: 
					if(cooldown++==25) {
						PIR_State=startDetection; cooldown=0; bStates=beginCountdown;LCD_Cursor(32);LCD_WriteData(0x07);
					}
					break;
				case ended: 
					if(howManyTimesTriggered==0) LCD_DisplayString(1,"Nothing was     detected!");
					else {
						LCD_DisplayString(1,"PIR triggered:");
						itoa(howManyTimesTriggered, dist,10);
						LCD_WriteString(17, dist); if (userTimerInput<10) LCD_WriteString(18, " ");LCD_WriteString(20, "time(s)");
					}
					bStates=ended2;
					break;
				case ended2:
					break;
				default: break;
			}
			break;
		//---------------------------------------------- C -----------------------------------------------------
		case SM1_C:
			if(KeypadChangingState == differentState) {
				LCD_WriteString(1, "2"); LCD_WriteData(0x02);
				LCD_WriteString(3, " 4"); LCD_WriteData(0x7F);
				LCD_WriteString(6, " 6"); LCD_WriteData(0x7E);
				LCD_WriteString(9, " 8"); LCD_WriteData(0x08);
				LCD_WriteString(12, " 5");  LCD_WriteData(0x05);
				LCD_WriteString(15, "                  ");
				PIR_State = pauseDetection;bStates=askTimer;
			}
			break;
		//---------------------------------------------- D -----------------------------------------------------
		case SM1_D:
			if(KeypadChangingState == differentState) {
				LCD_DisplayString(1, "Free movement!");
				PIR_State = pauseDetection;bStates=askTimer;
			}
			
			LCD_WriteString(17, dist);
			if (numbers<10) LCD_WriteString(18, " ");
			
			if(KeyPadChangingInput == differentInput)
				switch(KeypadInput) {
					case SM1_pound: LCD_WriteString(20, "Vertical"); break;
					case SM1_star: LCD_WriteString(20, "Horizontal"); break;
					default: break;
				}
			break;
		default: break;
	}
	
}
#endif