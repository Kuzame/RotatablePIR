/*!
 * **********************************************************************************************
 * \file test_sonar.c
 * \brief Testing sonar program using HC-SR04 Ultrasonic Sensor
 *
 * \author      :   Praveen Kumar
 * \date        :   Mar 24, 2014
 * Copyright(c) :   Praveen Kumar - www.veerobot.com
 * Description  :   Testing sonar program using HC-SR04 Ultrasonic Sensor
 *
 * LICENSE      :   Redistribution and use in source and/or binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain this copyright notice, list of conditions and disclaimer.
 * - Redistributions in binary form must reproduce this copyright notice, list of conditions and disclaimer in
 *      documentation and/or other materials provided with the distribution.
 *
 * DISCLAIMER   :   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" WITHOUT ANY
 * KIND OF WARRANTIES. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
 * **********************************************************************************************
 */
 
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "sonar.h"
 
int main(){
	DDRA = 0xFF;
	//DDRC = 0xFF;DDRB=0xFF;
    int distance_in_cm=0;
	unsigned char temp=0x00;
     // initialize display, cursor off
//    LCD_init();
//    LCD_DisplayString(1, "Distance(cm): ");
//  
    while(1)
    {
        distance_in_cm=read_sonar();
        if (distance_in_cm == TRIG_ERROR)
        {
            //LCD_DisplayString(1, "ERROR!!!");
            //_delay_ms(DELAY_BETWEEN_TESTS/2);
        }
        else if (distance_in_cm == ECHO_ERROR)
        {
            //LCD_DisplayString(1, "CLEAR!!!");
            //_delay_ms(DELAY_BETWEEN_TESTS);
        }
        else
        {
			//char dist[5];
			//itoa(distance_in_cm, dist, 10);
			//LCD_DisplayString(15, distance_in_cm);
            //LCD_WriteData(dist + '0');
			//LCD_WriteString(17, dist);
			
			if(distance_in_cm<64){
				temp|=distance_in_cm;
				PORTA = temp;
				temp=0x00;
			}
            _delay_ms(DELAY_BETWEEN_TESTS);
        }
    }
    return 0;
}
