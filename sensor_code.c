/*
File: sensor_code.c
Name: Swapnil Paratey
Date: 12/07/2014
Course: ECEN-5613 Embedded Systems Design Fall 2014
Subject: ESD Fall 2014 Final Project
Description: 
	AT89C51RC2 code for 11.0592MHz crystal sensor board code for RHT03/DHT22
	Used to receive Temperature, Humidity from sensor
	Transmit the sensor data serially to Zigbee chip
Usage:
	Just flash the code and interface the Sensor to Port 1.1
	Connect Zigbee to Serial interface
 */

#include <mcs51/8051.h>
#include <mcs51/at89c51ed2.h>
#include <stdio.h>
#include <mcs51/8052.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

unsigned char TempDataH;
unsigned char TempDataL;
unsigned char RHDataH;
unsigned char RHDataL;
unsigned char checksum;

int putstr(char *s);
void putchar(char c);
char getchar();
void serialinit();
void delay_ms(unsigned int k);
void StartSensor();

void main()
{
    serialinit();
    P1_0 = 0;

    while(1)
    {
        TempDataH = 0x00;
        TempDataL = 0x00;
        RHDataH = 0x00;
        RHDataL = 0x00;
        checksum = 0x00;
        
		// Acquire sensor data
        StartSensor();

		// Transmit sensor data one after another to Zigbee serially
        P1_0 = 1;
        putchar(TempDataH);
        delay_ms(1);
        putchar(TempDataL);
        delay_ms(1);
        putchar(RHDataH);
        delay_ms(1);
        putchar(RHDataL);
        delay_ms(1);
        putchar(checksum);
        delay_ms(1);
        P1_0 = 0;

		// Wait for a while before doing it again
        delay_ms(6000);
    }
}

// USED FOR TERMINAL DISPLAY STUFF
int putstr(char *s)
{
    int i = 0;
    while (*s)
    {
        putchar(*s++);
        i++;
    }
    putchar('\n');
    return i+1;
}

// Used for TERMINAL DISPLAY and to put data into Serial Buffer
void putchar(char c)
{
    // while (!TI);
    // while (TI == 0);
    while ((SCON & 0X02) == 0);
    SBUF = c;
    TI = 0;
}

// USED FOR TERMINAL DISPLAY STUFF
char getchar()
{
    // while (!RI);
    while ((SCON & 0x01) == 0);
    // while (RI == 0);
    RI = 0;
    return SBUF;
}

void serialinit()
{
    SCON = 0x50;
    TMOD = 0x20;
    TH1 = 0xFD;
    TR1 = 1;
    TI = 1;
    EA = 1;
    // EX0 = 1;
    // EA = 1;
}

void delay_10us()
{
    unsigned int i = 1;
	i++;i++;i++;i++;i++;i++;
}

void StartSensor()
{
    int i;
    unsigned char tempdata;
    tempdata = 0x00;
    P1_1 = 0;// P1_6 = 0; // P1_6 is just a pin used to understand when data is read and when it's stopped
    delay_ms(3);    // Host pulls low for 1ms
    P1_1 = 1;// P1_6 = 1;
    delay_10us();   // Host pulls high for 20-40ms
    delay_10us();
    delay_10us();
    delay_10us();
    delay_10us();
    delay_10us();
    delay_10us();
    P1_1 = 1;P1_6 = 0;  // Host keeps it high
    // while(P1_1);
    if(!P1_1)       // If Sensor drives port low, it has started an ACK
    {
        P1_6 = 1;
        while(!P1_1);   // Sensor drives low for a while 80us
        // P1_6 = 0;
        while(P1_1);    // Sensor drives high for a while 80us
        // Data transmission starts now
        // P1_6 = 1;	// Uncomment this to see in logic analyzer when data reading starts as a whole
        
		// Get the relative humidity high byte
		for(i=0;i<8;i++)
        {
            RHDataH = (RHDataH << 1);
            P1_6 = 0;
            while(P1_1);
            while(!P1_1);   // Data transmission starts by being low for 50us
            // P1_6 = 1;
            delay_10us();   // Host pulls high for 20-40ms
            delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            P1_6 = 1;	// Data reading here - Just read a bit and add it to the RHDataH
            if (!P1_1) RHDataH &= 0xFE;
            else RHDataH |= 0x01;
            P1_6 = 0;	// Data reading stops here
        }
        // RHDataH = tempdata;
        // tempdata = 0x00;
        // P1_6 = 0;
		
		// Get the relative humidity low byte
        for(i=0;i<8;i++)
        {
            RHDataL = (RHDataL << 1);
            while(P1_1);
            while(!P1_1);
            delay_10us();   // Host pulls high for 20-40ms
            delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            P1_6 = 1;	// Data reading here - Just read a bit and add it to the RHDataL
            if (!P1_1) RHDataL &= 0xFE;
            else if (P1) RHDataL |= 0x01;
            P1_6 = 0;	// Bit reading stops
        }
        // RHDataL = tempdata;
        // tempdata = 0x00;
        // P1_6 = 1;
		
		// Get the temperature high byte
        for(i=0;i<8;i++)
        {
            TempDataH = (TempDataH << 1);
            while(P1_1);
            while(!P1_1);
            delay_10us();   // Host pulls high for 20-40ms
            delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            P1_6 = 1;	// Indicate Bit reading start
            if (!P1_1) TempDataH &= 0xFE;
            else if (P1) TempDataH |= 0x01;
            P1_6 = 0;	// Indicate bit reading stop

        }
        // TempDataH = tempdata;
        // tempdata = 0x00;
        // P1_6 = 0;
		
		// Get the Temperature Low Byte
        for(i=0;i<8;i++)
        {
            TempDataL = (TempDataL << 1);
            while(P1_1);
            while(!P1_1);
            delay_10us();   // Host pulls high for 20-40ms
            delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            P1_6 = 1;	// Bit reading start
            if (!P1_1) TempDataL &= 0xFE;
            else if (P1) TempDataL |= 0x01;
            P1_6 = 0;	// Bit reading stop
        }
        // TempDataL = tempdata;
        // tempdata = 0x00;
        // P1_6 = 1;
		
		// Get the checksum
        for(i=0;i<8;i++)
        {
            checksum = (checksum << 1);
            while(P1_1);
            while(!P1_1);
            delay_10us();   // Host pulls high for 20-40ms
            delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            // delay_10us();
            P1_6 = 1;	// Bit reading start
            if (!P1_1) checksum &= 0xFE;
            else if (P1) checksum |= 0x01;
            P1_6 = 0;	// Bit reading stop
        }
        // checksum = tempdata;
        // tempdata = 0x00;
    }
    P1_1 = 1;
    P1_6 = 0;
}

void delay_ms(unsigned int k) //delay in milliseconds for 11.0592MHz crystal - THIS IS FOR LCD ONLY
{                                            //E.g. delay_ms(100)
    unsigned int i,j;
    for(i=0;i<k;i++)
    {
        for(j=0;j<110;j++);
    }
}

