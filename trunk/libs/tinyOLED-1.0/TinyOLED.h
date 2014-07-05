/*
TinyOLED - a small library for OLED 0.96
Copyright (C) 2014 Olivier Gaillard
All rights reserved.
*/

#ifndef __TinyOLED_h
#define __TinyOLED_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define COMMAND_MODE		0x80
#define DATA_MODE			0x40
#define DISPLAY_OFF			0xAE
#define DISPLAY_ON			0xAF
#define H_ADDRESSING		0x00
#define P_ADDRESSING		0x02
#define NORMAL_DISPLAY		0xA6
#define INVERSE_DISPLAY		0xA7
#define SET_BRIGHTNESS		0x81

class TinyOLED
{
  public:
    TinyOLED(int oledAddress);
	void sendCmd(byte command);
	void sendData(byte Data);
	void turnOn();
	void turnOff();
	void setPMode();
	void setHMode();
	void cls();
	void init();
	void setXY(byte Column, byte Row);
	void clear();
    void drawChar(char c);
	void drawString(const char *str);
	void drawStringXY(byte x, byte y, char *str);
	void drawBitmap(byte x, byte y, const byte *bitmap, byte w=128, byte h=96);
	void drawBigNums(int x, int y, char *string);
	void drawBigNum(int x, int y, char car);
  private:
	int _oledAdrress;
    void send(int mode, byte command);

};


#endif 