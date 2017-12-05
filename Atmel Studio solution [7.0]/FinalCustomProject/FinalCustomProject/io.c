#include <avr/io.h>
//#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/sfr_defs.h>
#include "io.h"

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))


#define datapin     1 	// define datapin from atmega1284 [4]
#define Epin		2	// Enable pin [6]
/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTB	// port connected to pins 4 and 6 of LCD disp.
#define RS 1//6			// Register select signal. HIGH = Data Input, L = Instruction Input. AKA: Data pin [4]
#define E 2//7			// Clock pin [6]

/*-------------------------------------------------------------------------*/

void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_init(void) {

	//wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);
}

void LCD_WriteCommand (unsigned char Command) {
	CLR_BIT(CONTROL_BUS,RS);
	DATA_BUS = Command;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_WriteData(unsigned char Data) {
	SET_BIT(CONTROL_BUS,RS);
	DATA_BUS = Data;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(1);
}

void LCD_WriteString( unsigned char column, const unsigned char *string) {
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}

void LCD_DisplayString( unsigned char column, const unsigned char* string) {
	LCD_ClearScreen();
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x1 LCD: column < 9
		// 16x2 LCD: column < 17
		LCD_WriteCommand(0x80 + column - 1);
		} else {
		LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
		// 16x2 LCD: column - 9
	}
}

void delay_ms(int miliSec) //for 8 Mhz crystal

{
	int i,j;
	for(i=0;i<miliSec;i++)
	for(j=0;j<775;j++)
	{
		asm("nop");
	}
}

// Special thanks to:	http://www.scienceprog.com/creating-custom-lcd-characters-using-winavr/
// and:					http://www.scienceprog.com/connect-lcd-to-atmega-using-3-wires/
void LCDdefinechar(const uint8_t *pc,uint8_t char_code){
	uint8_t a, pcc;
	uint16_t i;
	a=(char_code<<3)|0x40;
	for (i=0; i<8; i++){
		pcc=pgm_read_byte(&pc[i]);
		LCDwritebyte(a++,pcc);
	}
}
void LCDwritebyte(uint8_t a, uint8_t pc)
{
	LCDsendCommand(a);
	LCDsendChar(pc);
}
void LCDsendCommand(uint8_t cmd) //forms data ready to send to 74HC164
{
	sendByteToRegister(cmd);//sends command to shift register
	LCDenableCommand();
	delay_ms(1);
	LCDdisableCommand();
}
void LCDsendChar(uint8_t letter) //forms data ready to send to 74HC164
{
	sendByteToRegister(letter);//sends char to shift register
	LCDenableData();
	delay_ms(1);
	LCDdisableData();
}
void LCDenableCommand()
{
	CONTROL_BUS |= _BV(Epin);
}
void LCDdisableCommand()
{
	CONTROL_BUS &= ~_BV(Epin);
}
void LCDenableData()
{
	CONTROL_BUS |= _BV(datapin);
	CONTROL_BUS |= _BV(Epin);
}
void LCDdisableData()
{
	CONTROL_BUS &= ~_BV(Epin);
	CONTROL_BUS &= ~_BV(datapin);
}

void sendByteToRegister(uint8_t LCDdata) // HEAVILY MODIFIED (compared to websites)
{
	uint8_t i, temp;
	CONTROL_BUS&=~_BV(datapin);			// sets datapin to output a LOW
	temp=LCDdata;
	DATA_BUS = LCDdata;
	delay_ms(1);
	CONTROL_BUS&=~_BV(datapin);	// sets datapin to output a LOW
}