/* TinyOLED - a small library for OLED 0.96
 Copyright (C) 2014 Olivier Gaillard
 All rights reserved. */

#include <Wire.h>
#include <TinyOLED.h>

TinyOLED tinyOLED(0x3C); // SPI Address
char buffer[6];
String number = "";
long i = 0;  

void setup(){
  tinyOLED.init();  //init OLED display
}

void loop(){
  number = String(i++, DEC);
  number.toCharArray(buffer, 6);
  tinyOLED.drawBigNums(15-(number.length()*3), 2, buffer);
  if (i > 99999) i = 0;
}