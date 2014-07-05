/* TinyOLED - a small library for OLED 0.96
 Copyright (C) 2014 Olivier Gaillard
 All rights reserved. */

#include <Wire.h>
#include <TinyOLED.h>

TinyOLED tinyOLED(0x3C); // SPI Address
char buffer [12];
long i = 0;  

void setup(){
  tinyOLED.init();  //init OLED display
}

void loop(){
  ltoa(i++, buffer, 10);
  tinyOLED.drawBigNums(0, 2, buffer);
  if (i > 99999) i = 0;
}
