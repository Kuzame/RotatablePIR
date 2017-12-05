/*
 * FinalCustomProject.c
 *
 * Created: 6/1/2017 3:25:08 PM
 * Author : AdrianH
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "Scheduler.h"
#include "pwm.h"
#include "Servo.h"
#define F_CPU 8000000UL
//Shared variable
//Easy custom char maker: http://maxpromer.github.io/LCD-Character-Creator/
const uint8_t love[] PROGMEM= {
	0x00,	0x00,	0x0A,	0x1F,	0x1F,	0x0E,	0x04,	0x00
};
const uint8_t dota[] PROGMEM= {
	0x00,	0x00,	0x11,	0x1C,	0x0F,	0x07,	0x13,	0x00
};
const uint8_t Pause [] PROGMEM= {
	0x1B,	0x1B,	0x1B,	0x1B,	0x1B,	0x1B,	0x1B,	0x00
};
const uint8_t Play[] PROGMEM= {
	0x18,	0x1C,	0x1E,	0x1F,	0x1E,	0x1C,	0x18,	0x00
};
const uint8_t Up[] PROGMEM= {
	0x00,	0x04,	0x0E,	0x15,	0x04,	0x04,	0x04,	0x00
};
const uint8_t Down[] PROGMEM= {
	0x00,	0x04,	0x04,	0x04,	0x15,	0x0E,	0x04,	0x00
};
const uint8_t Mid[] PROGMEM= {
	0x00,	0x00,	0x04,	0x00,	0x15,	0x00,	0x04,	0x00
};

int main()
{
	// Set Data Direction Registers
	// For buttons,								DDR needs to be 0, while PORT needs to be 1
	// For PIR sensor,							DDR needs to be 0, and PORT also 0
	// For outputs (LCD data & control bus),	DDR needs to be 1, while PORT needs to be 0
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xF7; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00; // LCD control lines
	
	//DDRB = 0x00; PORTB = 0x00;
	//DDRC = 0xF0; PORTC = 0x0F; // #### this is for GetKeyPad ####
	//DDRD = 0xFF; PORTD = 0x00; 
	// . . . etc


	// Period for the tasks
	unsigned long int SMTick1_calc = 100;
	unsigned long int SMTick2_calc = 100;
	unsigned long int SMTick3_calc = 50;
	unsigned long int SMTick4_calc = 50;
	unsigned long int SMTick5_calc = 50;
	unsigned long int SMTick6_calc = 100;

	//Calculating GCD
	unsigned long int tmpGCD = 50;
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	unsigned long int SMTick4_period = SMTick4_calc/GCD;
	unsigned long int SMTick5_period = SMTick5_calc/GCD;
	unsigned long int SMTick6_period = SMTick5_calc/GCD;
	
	// Sorted according to its PRIORITY
	// 3 Keypad
	// 1 PIR
	// 2 Alarm
	// 4 Distance sensor
	// 5 Servomotor
	// 6 LCD
	//Declare an array of tasks
	static task scanning, alarm, lcd, keypad, servo, sonar;
	task *tasks[] = { &keypad, &scanning, &alarm, &sonar, &servo,&lcd, };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	
	// Getting input from KeyPad
	keypad.state = -1;//Task initial state.
	keypad.period = SMTick1_period;//Task Period.
	keypad.elapsedTime = SMTick1_period;//Task current elapsed time.
	keypad.TickFct = &SMTick1;//Function pointer for the tick.
	// Scanning with PIR
	scanning.state = -1;//Task initial state.
	scanning.period = SMTick2_period;//Task Period.
	scanning.elapsedTime = SMTick2_period;//Task current elapsed time.
	scanning.TickFct = &SMTick2;//Function pointer for the tick.
	// Trigerring the alarm
	alarm.state = -1;//Task initial state.
	alarm.period = SMTick3_period;//Task Period.
	alarm.elapsedTime = SMTick3_period;//Task current elapsed time.
	alarm.TickFct = &SMTick3;//Function pointer for the tick.
	// Ultrasonar sensor
	sonar.state = -1;//Task initial state.
	sonar.period = SMTick4_period;//Task Period.
	sonar.elapsedTime = SMTick4_period;//Task current elapsed time.
	sonar.TickFct = &SMTick4;//Function pointer for the tick.
	// Moving Servomotor
	servo.state = -1;//Task initial state.
	servo.period = SMTick5_period;//Task Period.
	servo.elapsedTime = SMTick5_period;//Task current elapsed time.
	servo.TickFct = &SMTick5;//Function pointer for the tick.
	// Displaying the LCD
	lcd.state = -1;//Task initial state.
	lcd.period = SMTick6_period;//Task Period.
	lcd.elapsedTime = SMTick6_period;//Task current elapsed time.
	lcd.TickFct = &SMTick6;//Function pointer for the tick.



	// -----------	Init	------------------
	TimerSet(GCD);
	TimerOn();
	PWM_on();
	LCD_init();
	LCDdefinechar(dota, 0x03);
	LCDdefinechar(Up, 0x02);
	LCDdefinechar(love, 0x04);
	LCDdefinechar(Mid, 0x05);
	LCDdefinechar(Pause, 0x06);
	LCDdefinechar(Down, 0x08);
	LCDdefinechar(Play, 0x07);
	servo_init();
	initPosition();
	
	unsigned i;
	LCD_DisplayString(1, "Rotatable PIR "); LCD_WriteData(0x07);  LCD_WriteString(17, "Adrian Harminto");
	
	while(1) {
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 0;
}
