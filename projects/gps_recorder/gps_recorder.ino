#include <Wire.h>
#include <TinyOLED.h>
#include <Button.h>
#include <TinyGPS++.h>
#include <AltSoftSerial.h>
#include "data.h"

#define DEBUGMODE        1
#define GPS_BAUD 9600

#define OLED_address  0x3c
#define BUTTON1_PIN     4
#define KMH     0
#define METER   1
#define DEG     2
#define NB_SCREENS 5

AltSoftSerial ss; // TX_PIN 9 - RX_PIN 8
TinyGPSPlus gps;
TinyOLED tinyOLED(0x3C); // SPI Address

// GND -----/ button ------ pin 4
Button button1 = Button(BUTTON1_PIN,BUTTON_PULLUP_INTERNAL);
boolean longPush = false;
boolean topSec, lastTop = true; // top des secondes
boolean gpsFix = false; 
int screen = 1, lastScreen = 0;
char buf [16];
char todayDate[10];
boolean refresh = true; // refresh display
static unsigned long lWaitMillis;
byte gpsHour, gpsMinute = 0;
char* labels[5] = {"ATOMIC CLOCK", "ALTITUDE", "SPEED", "LOCATION", "COURSE"};
unsigned long fixAge = 999999; // age of last fix in milliseconds
byte nbSats = 0; // number of sats
//float gpsLat, gpsLong;  

/* ----- Main program ----- */
void setup() {
  if (DEBUGMODE) {
    Serial.begin(115200);
    Serial.println("GPS Recorder");
  }
  ss.begin(GPS_BAUD);
  // Passage à 4800 bauds
  //ss.println("$PUBX,41,1,0007,0003,4800,0*13"); 
  //ss.begin(GPS_BAUD);
  ss.flush();

  // Init button
  button1.releaseHandler(handleButtonReleaseEvents);
  //button1.releaseHandler(handleButtonPressEvents);
  button1.holdHandler(handleButtonHoldEvents,2000);

  // Initialize I2C and OLED Display
  Wire.begin();
  tinyOLED.init();  //init OLED display
  displaySplash();
  delay(700);
  tinyOLED.cls();
  lWaitMillis = millis() + 1000;  // initial setup
}

void loop() {  
 
  button1.isPressed();

  /* -------- GPS ----------- */
   if (gps.location.isValid()) {
      fixAge =  gps.location.age();
   }
  

  // top des secondes
  if( (long)( millis() - lWaitMillis  ) >= 0) {
    lWaitMillis += 1000;
    topSec = !topSec;
    showFix(); // update fix picto
    if (gps.satellites.isUpdated()) nbSats = gps.satellites.value();
    if (gps.time.isValid()) {
      gpsHour = gps.time.hour()+2; // heure d'été
      gpsMinute = gps.time.minute();
      if (gpsHour > 23) gpsHour-=24;
      }
    
    if (gps.date.isValid()) {
      sprintf (todayDate, "%02d/%02d/%04d",  gps.date.day(), gps.date.month(), gps.date.year());
    }
    //debugSerial("Nb Sats", (gps.satellites.isValid())?gps.satellites.value():-1);
    //debugSerial("Hdop", (gps.hdop.isValid())?gps.hdop.value():-1);
    //debugSerial("chars processed", gps.charsProcessed());
  }

  if (screen != lastScreen) {
    refresh = true;
    lastScreen = screen;
  }
  if (refresh) initScreen(screen);
  
  switch(screen) {
    case 1 : // Clock
    if (lastTop != topSec || refresh) {
      showClock(topSec);
      showDate();
      lastTop = topSec;
    }
    break;

  case 2 : // Altitude
    if (gps.altitude.isUpdated()) {
      showAltitude(abs(gps.altitude.meters()));
    }
    break;

  case 3 : // Speed
    if (gps.speed.isUpdated()) {
      showSpeed(gps.speed.kmph());
    }    
    break;

  case 4 : // Location
     if (gps.location.isUpdated()) {
      showLocation(gps.location.lat(), gps.location.lng());
    }
    break;

  case 5 : // Course
    if (gps.course.isUpdated()) {
      if (gps.course.deg() <= 360) showCourse((int)gps.course.deg());
    }
    break;
  }
  refresh = false;
  smartDelay(50);
  
  /*while (!feedgps()) {
    delay(20);
  }*/
 
 if (millis() > 5000 && gps.charsProcessed() < 10) {
   if (DEBUGMODE) Serial.println(F("No GPS data received !"));
     tinyOLED.drawStringXY(3,3, "No GPS data"); 
     tinyOLED.drawStringXY(3,5, " received !"); 
 }

}

/* ----- Managing screen display ----- */
void initScreen(int num) {
  showFix();
  tinyOLED.drawBitmap(2, 0, header, 72, 8);
  showBatterylevel(readVcc());
  // clear row 2 -> 8
  for(byte row=2; row<8; row++) {	
    tinyOLED.setXY(0,row);    
    for(byte col=0; col<128; col++) tinyOLED.sendData(0);
  }
  if(num == 1) showDate();
  else tinyOLED.drawStringXY((16-strlen(labels[num-1]))/2 ,7, labels[num-1]);
}
void showDate(){
  sprintf (buf, "[%02d] %s", nbSats, todayDate);
  tinyOLED.drawStringXY(0,7, buf); 
}
void showSpeed(int s) {
  sprintf (buf, "%4i", s);
  tinyOLED.drawBigNums(0,2, buf);
  tinyOLED.drawBitmap(12, 4, units[KMH], 32, 16);
}
void showFix() {
  tinyOLED.drawBitmap(0, 0, pictofix[(fixAge <2000)?0:1], 8, 8);
  debugSerial("fixAge", fixAge);
}
void showAltitude(int alti) {
  sprintf (buf, "%4i", alti);
  tinyOLED.drawBigNums(0,2, buf);
  tinyOLED.drawBitmap(12, 4, units[METER], 32, 16);
  debugSerial("altitude", alti);
}
void showLocation(float lat, float lng) {
  dtostrf(lat, 8, 6, buf);
  tinyOLED.drawStringXY(0,2, buf);
  dtostrf(lng, 8, 6, buf);
  tinyOLED.drawStringXY(0,4, buf);
}

void showCourse(int c) {
  sprintf (buf, "%3i", c);
  tinyOLED.drawBigNums(3,2, buf);
  //tinyOLED.drawBitmap(10, 4, units[DEG], 32, 16);
  debugSerial("course", c);
}

void showClock(boolean topSec) {
  if (topSec) sprintf (buf, "%02i:%02i", gpsHour,gpsMinute);
  else sprintf (buf, "%02i;%02i", gpsHour, gpsMinute);
  tinyOLED.drawBigNums(0,2, buf);
  debugSerial("gpsHour", gpsHour);
  debugSerial("gpsMinute", gpsMinute);
}

void displaySplash(void) {
 tinyOLED.drawBitmap(0, 0, splash, 128, 64);
}

/* ----- Viewing battery level ----- */
void showBatterylevel(long vcc) {
  byte level = 0;
  if (vcc > 3600) level=4; 
  else if (vcc > 3400) level=3; 
  else if (vcc > 3200) level=2; 
  else if (vcc > 3000) level=1; 
  tinyOLED.drawBitmap(14, 0, battery[level], 16, 8); 
  debugSerial("Batterylevel", vcc);
}
// Read VCC
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

/* ---------- Gestion du bouton ---------- */

// Bouton relaché
void handleButtonReleaseEvents(Button &btn) {
  screen++;
  if (screen > NB_SCREENS) screen = 1;
}

// Appui prolongé sur le bouton
void handleButtonHoldEvents(Button &btn) {
  //if (DEBUGMODE) Serial.println("Hold");

}

void handleButtonPressEvents(Button &btn) {
  //if (DEBUGMODE) Serial.println("Press");
}


/* ------------ GPS -------------- */

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available()) {
      char c = ss.read();
      //Serial.write(c); 
      gps.encode(c);
    }
  } while (millis() - start < ms);
}

bool feedgps()
{
  while (ss.available())
  {
    if (gps.encode(ss.read()))
      return true;
  }
  return false;
}


void debugSerial(char *label, int value) {
   if (DEBUGMODE) {
     Serial.print(label);
     Serial.print(" = ");
     Serial.println(value); 
     }
}




