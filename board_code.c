/*
File: boardcode.c
Name: Swapnil Paratey
Date: 12/07/2014
Course: ECEN-5613 Embedded Systems Design Fall 2014
Subject: ESD Fall 2014 Final Project
Description: 
	Development board code for Final Project
	Used to receive Temperature, Humidity from Zigbee chip
	Display temperature, humidity and RSSI
Usage:
	Needs SPLD code for LCD interface - SPLD WinCUPL code attached with submission
	Has Memory mapped LCD interfacing
 */

// Include files
#include <mcs51/8051.h>
#include <mcs51/at89c51ed2.h>
#include <stdio.h>
#include <mcs51/8052.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// LCD values used for memory mapped I/O
#define LCDUNLOCK 0x30
#define LCDFUNCSET 0x38
#define LCDDISPLAYOFF 0x08
#define LCDDISPLAYON 0x0C
#define LCDENTRYMODE 0x06
#define LCDCLRSCREEN 0x01
#define LCDCURSORBLINK 0x0E

// Global pointers can be accessed everywhere
__xdata __at (0x8000) unsigned char dbvalue;    // Memory Mapped LCD for E = 1, RS = 0, RW = 0
__xdata __at (0xC000) unsigned char busyflag;   // Memory Mapped LCD for E = 1, RS = 0, RW = 1
__xdata __at (0xA000) unsigned char writedata;  // Memory mapped LCD for writing data E=1, RS=1, RW=0
__xdata __at (0xE000) unsigned char readdata;   // Memory mapped LCD for reading data E=1, RS=1, RW=1

// Defined as global pointers during start of development, but can be defined locally inside main() too
unsigned char TempDataH;
unsigned char TempDataL;
unsigned char RHDataH;
unsigned char RHDataL;
unsigned char checksum;

void serialinit();	// Serial communication interfacing code for 8051 with PC terminal and Zigbee
void delay_ms(unsigned int k);	// Function used for creating delay for LCD
void lcdinit();	// LCD init function
void lcdbusywait();	// To check for LCD being busy and not pass any data/instruction
void lcdgotoaddr(unsigned char addr);	// Used to head to an DDRAM address
void lcdgotoxy(unsigned char row, unsigned char column);	// Simplified function for heading to x and y location in LCD
void lcdputch(char cc);	// Used to display a char on an LCD location
void lcdputstr(char *ss);	// Used to display a series of char on LCD location	
unsigned char HexToHex(unsigned char input);	// Used to convert integer chars to Hex values, hard coded returns

void main()
{
    char *ESDLine = "ESDF14 Final";
    char *TempLine = "Temp:   . C";
    char *RHLine = "RH:   . %";
    char *RSSILine = "RSSI: -  dBm";
    int tempdec, tempint, rhdec, rhint, val_tens, val_units, val_dec, rssi_units, rssi_tens, rssi_value;
    unsigned char cvals_tens, cvals_units, cvals_dec, cval_rssi_H, cval_rssi_L, rssi;
    unsigned char odie, tempbuf;
    odie = 0x0D;

    serialinit();
    delay_ms(20);
    lcdinit();
    P1_0 = 0;

	// Write initial lines
    lcdgotoxy(0,0);
    lcdputstr(TempLine);
    lcdgotoxy(1,0);
    lcdputstr(RHLine);
    lcdgotoxy(2,0);
    lcdputstr(RSSILine);
    lcdgotoxy(3,0);
    lcdputstr(ESDLine);

	// while loop
    while(1)
    {
        TempDataH = 0x00;
        TempDataL = 0x00;
        RHDataH = 0x00;
        RHDataL = 0x00;
        checksum = 0x00;
        rssi = 0x00;

        TempDataH = getchar();
        TempDataL = getchar();
        RHDataH = getchar();
        RHDataL = getchar();
        checksum = getchar();

        // Temperature calculations and printout on LCD
        if((TempDataH & 0x80) == 0x80)
        {
            lcdgotoxy(0,5);
            lcdputch('-');
            TempDataH = (TempDataH & 0x7F);
        }
        else
        {
            lcdgotoxy(0,5);
            lcdputch(' ');
        }
        tempint = ((int)TempDataH);
        tempdec = ((int)TempDataL);
        tempint = (tempint << 8);
        tempint = tempint + tempdec;

        val_tens = tempint/100;
        val_units = (tempint/10)%10;
        val_dec = tempint%10;

        cvals_tens = (char)val_tens;
        cvals_units = (char)val_units;
        cvals_dec = (char)val_dec;

        lcdgotoxy(0,6);
        lcdputch(cvals_tens+48);

        lcdgotoxy(0,7);
        lcdputch(cvals_units+48);

        lcdgotoxy(0,9);
        lcdputch(cvals_dec+48);

        // Humidity calculations and printout on LCD
        rhint = ((int)RHDataH);
        rhdec = ((int)RHDataL);
        rhint = (rhint << 8);
        rhint = rhint + rhdec;

        val_tens = rhint/100;
        val_units = (rhint/10)%10;
        val_dec = rhint%10;

        cvals_tens = (char)val_tens;
        cvals_units = (char)val_units;
        cvals_dec = (char)val_dec;

        lcdgotoxy(1,4);
        lcdputch(cvals_tens+48);
        lcdgotoxy(1,5);
        lcdputch(cvals_units+48);
        lcdgotoxy(1,7);
        lcdputch(cvals_dec+48);

		// Process for getting RSSI from Zigbee - Send AT command and receive a Hex value
        putchar('+');putchar('+');putchar('+'); // Sending AT command to Zigbee to ask
        tempbuf = getchar();    // Receive O
        tempbuf = getchar();    // Receive K
        tempbuf = getchar();    // Received 0x0D
        putchar('A');putchar('T');putchar('D');putchar('B');putchar(odie);   // Sending ATDB
        cval_rssi_H = getchar();	// Received first value of RSSI in Hex
        cval_rssi_L = getchar();	// Received second value of RSSI in Hex
        tempbuf = getchar();    // Received 0x0D
        putchar('A');putchar('T');putchar('C');putchar('N');putchar(odie);   // Sending ATCN
        tempbuf = getchar();    // Receive O
        tempbuf = getchar();    // Receive K
        tempbuf = getchar();    // Received 0x0D

        rssi = HexToHex(cval_rssi_H);
        rssi = (rssi << 4);
        rssi = rssi | HexToHex(cval_rssi_L);

        rssi_value = (int)rssi;
        rssi_tens = rssi_value/10;
        rssi_units = rssi_value%10;

        cval_rssi_H = (char)rssi_tens;
        cval_rssi_L = (char)rssi_units;

		// Printing RSSI in the appropriate location
        lcdgotoxy(2,7);
        lcdputch(cval_rssi_H+48);
        lcdgotoxy(2,8);
        lcdputch(cval_rssi_L+48);
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

// USED FOR TERMINAL DISPLAY STUFF
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

void delay_ms(unsigned int k) //delay in milliseconds for 11.0592MHz crystal - THIS IS FOR LCD ONLY
{                                            //E.g. delay_ms(100)
    unsigned int i,j;
    for(i=0;i<k;i++)
    {
        for(j=0;j<110;j++);
    }
}

void lcdinit()
{
    delay_ms(20); 			// Wait for more than 15ms
    dbvalue = LCDUNLOCK; 	// Write defines for these common instructions to make code readable
    delay_ms(5);
    dbvalue = LCDUNLOCK;
    delay_ms(1);
    dbvalue = LCDUNLOCK;
    lcdbusywait();
    dbvalue = LCDFUNCSET;
    lcdbusywait();
    dbvalue = LCDDISPLAYOFF;
    lcdbusywait();
    dbvalue = LCDDISPLAYON;
    lcdbusywait();
    dbvalue = LCDENTRYMODE;
    lcdbusywait();
    dbvalue = LCDCLRSCREEN;
}

void lcdbusywait()
{
	// Check for Port 0.7 bit high or not
    while((busyflag & (0x80)) == 0x80);
}

void lcdgotoaddr(unsigned char addr)
{
    lcdbusywait();
	// used to set DB7 as 1 always ... refer datasheet Pg. 191 HD44780U
    dbvalue = addr; 
}

void lcdgotoxy(unsigned char row, unsigned char column)
{
    switch(row)
    {
        case 0:
            row = 0x80; // setting row = 0 and DB7 = 1
            lcdgotoaddr(row+column);
            break;
        case 1:
            row = 0xC0; // setting row = 4 and DB7 = 1
            lcdgotoaddr(row+column);
            break;
        case 2:
            row = 0x90; // setting row = 1 and DB7 = 1
            lcdgotoaddr(row+column);
            break;
        case 3:
            row = 0xD0; // setting row = 5 and DB7 = 1
            lcdgotoaddr(row+column);
            break;
        default: // choose the first row for any random garbage coming into function parameters
            row = 0x80; // setting row = 0 and DB7 = 1
            lcdgotoaddr(row+column);
            break;
    }
}

void lcdputch(char cc)
{
    lcdbusywait();
    writedata = cc;
}

void lcdputstr(char *ss)
{
    unsigned int i;
    // Add new line handles
    for(i = 0; ss[i] != '\0'; i++)
    {
        lcdputch(ss[i]);
    }
}

// Function used to make it easy to Hex/char values of integers to Hex
unsigned char HexToHex(unsigned char input)
{
    unsigned char result;
    switch(input)
    {
        case 0x30 : result = 0x00; break;
        case 0x31 : result = 0x01; break;
        case 0x32 : result = 0x02; break;
        case 0x33 : result = 0x03; break;
        case 0x34 : result = 0x04; break;
        case 0x35 : result = 0x05; break;
        case 0x36 : result = 0x06; break;
        case 0x37 : result = 0x07; break;
        case 0x38 : result = 0x08; break;
        case 0x39 : result = 0x09; break;
        case 0x40 : result = 0x0A; break;
        case 0x41 : result = 0x0B; break;
        case 0x42 : result = 0x0C; break;
        case 0x43 : result = 0x0D; break;
        case 0x44 : result = 0x0E; break;
        case 0x45 : result = 0x0F; break;
        default : result = 0x00; break;
    }
    return result;
}
