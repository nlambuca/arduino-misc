#include <Wire.h>
#include <TinyOLED.h>
#include <Button.h>
#include "data.h"

#define DEBUGMODE        1
// OLED I2C bus address
#define OLED_address  0x3c
#define BUTTON1_PIN     4
#define KMH     0
#define METER   1
#define DEG     2
#define NB_SCREENS 5

TinyOLED tinyOLED(0x3C); // SPI Address

// GND -----/ button ------ pin 4
Button button1 = Button(BUTTON1_PIN,BUTTON_PULLUP_INTERNAL);
boolean longPush = false;
boolean topSec = true; // top des secondes
boolean gpsFix = true; 
int screen = 0, lastScreen = 0;
char buf [4];
boolean refresh = true; // refresh display
static unsigned long lWaitMillis;
int altitude, lastAltitude = 0;
int speed, lastSpeed = 0;
int minTime, lastMinTime = 0;
char* labels[5] = {"ATOMIC CLOCK", "SPEED", "ALTITUDE", "LOCATION", "HEADING"};

void setup() {
/*  if (DEBUGMODE) {
    Serial.begin(57600);
  }*/
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

  altitude = 4807;
  speed = 168;
  minTime = 30;
  lWaitMillis = millis() + 1000;  // initial setup
}

void loop() {  
  char buf [4];
  
  button1.isPressed();
  if (!gpsFix) screen = 0;
  delay(10);
  if( (long)( millis() - lWaitMillis  ) >= 0) {
    lWaitMillis += 1000;
    topSec = !topSec;
    altitude += random(-2, 3);
    speed += random(-1, 2);
    minTime += random(1, 3);
    if (DEBUGMODE) Serial.println("update Data");
  }

  if (screen != lastScreen) {
    refresh = true;
    lastScreen = screen;
  }
  if (refresh) initScreen(screen);
  
  switch(screen) {
  case 0 : // noFix
    if (lastMinTime != minTime) {
        tinyOLED.drawStringXY(4,3, "Waiting"); 
        tinyOLED.drawStringXY(2,5, "for GPS data"); 
        lastMinTime = minTime;
    }
    break;
    case 1 : // Clock
    if (lastMinTime != minTime) {
      showClock(topSec);
      lastMinTime = minTime;
    }

    break;

  case 2 : // Altitude
    if (lastAltitude != altitude) {
      showAltitude(abs(altitude));
      lastAltitude = altitude;
    }
    break;

  case 3 : // Speed
    if (lastSpeed != speed) {
      showSpeed(speed);
      lastSpeed = speed;
    }    
    break;

  case 4 : // Location
    showLocation();
    break;

  case 5 : // Heading
    showHeading(180);
    break;
  }
  refresh = false;
}

/* -------- Gestion de l'affichage -------- */
void initScreen(int num) {
  tinyOLED.drawBitmap(0, 0, pictofix[(gpsFix==true)?0:1], 8, 8);
  tinyOLED.drawBitmap(2, 0, header, 72, 8);
  showBatterylevel(readVcc());
  // clear row 2 -> 8
  for(byte row=2; row<8; row++) {	
    tinyOLED.setXY(0,row);    
    for(byte col=0; col<128; col++) tinyOLED.sendData(0);
  }
  if(num > 0) tinyOLED.drawStringXY((16-strlen(labels[num-1]))/2 ,7, labels[num-1]);
}

void showSpeed(int s) {
  sprintf (buf, "%04i", s);
  tinyOLED.drawBigNums(0,2, buf);
  tinyOLED.drawBitmap(12, 4, units[KMH], 32, 16);
}

void showAltitude(int a) {
  sprintf (buf, "%04i", a);
  tinyOLED.drawBigNums(0,2, buf);
  tinyOLED.drawBitmap(12, 4, units[METER], 32, 16);
}
void showLocation() {
  tinyOLED.drawStringXY(0,2, "45.6541358"); 
  tinyOLED.drawStringXY(0,4, "03.2458689"); 
}

void showHeading(int h) {
  sprintf (buf, "%03i", h);
  tinyOLED.drawBigNums(1,2, buf);
  tinyOLED.drawBitmap(10, 4, units[DEG], 32, 16);
}

void showClock(boolean topSec) {
  if (topSec == true) sprintf (buf, "%02i:%02i", 18,55);
  else sprintf (buf, "%02i;%02i", 18,55);
  tinyOLED.drawBigNums(0,2, buf);
}

void displaySplash(void) {
 tinyOLED.drawBitmap(0, 0, splash, 128, 64);
}

/* ---- */
// Show battery level icon
void showBatterylevel(long vcc) {
  byte level = 0;
  if (vcc > 3600) level=4; 
  else if (vcc > 3400) level=3; 
  else if (vcc > 3200) level=2; 
  else if (vcc > 3000) level=1; 
  tinyOLED.drawBitmap(14, 0, battery[level], 16, 8); 
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

/* ---------- Gestion du boutons ---------- */

// Bouton relaché
void handleButtonReleaseEvents(Button &btn) {
  if (gpsFix) screen++;
  if (screen > NB_SCREENS) screen = 1;
}

// Appui prolongé sur le bouton
void handleButtonHoldEvents(Button &btn) {
  //if (DEBUGMODE) Serial.println("Hold");
  /*
  longPush = true;
   screen = 1;
   value = 0;
   if (screen == 1 && ++etat > 2) {
   etat = 0;
   delay(500);
   }
   else if (screen == 2 || screen == 3) {
   resetAltiMinMax();
   }
   */
}

void handleButtonPressEvents(Button &btn) {
  //if (DEBUGMODE) Serial.println("Press");
}












