/*
TinyOLED - a small library for OLED 0.96
Copyright (C) 2014 Olivier Gaillard
All rights reserved.
*/

#include <Wire.h>
#include <TinyOLED.h>

TinyOLED tinyOLED(0x3C); // SPI Address

void setup(){
  tinyOLED.init();  //init OLED display
  tinyOLED.drawStringXY(3,3, "It works !"); 	// draw a test string
}

void loop(){
}