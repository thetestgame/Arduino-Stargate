// Include the Motor shield and NeoPixel Libraries
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels
#define PIN 6

// How many NeoPixels are attached to the Arduino
#define NUMPIXELS 9

// How many steps are required to advance the Symbol Ring by one symbol *needs tweaking
int STEPPERSYM = 31;

// Define the Pixel strand item "pixels"
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// Define the colours to use for the NeoPixels
uint32_t orange = pixels.Color(255,40,5);
uint32_t off = pixels.Color(0,0,0);
 
// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

// Define Stargate address to be dialed as array
// These numbers are the references to the Gate symbols, including the point of origin (1)
// reference can be found here http://rdanderson.com/stargate/glyphs/glyphs.htm 

int SGAddress[] = {27,7,15,32,12,30,1};

// Define delay for Blinking
int BlinkDelay = 1500; 

//Stores the last dial direction
int lastDir = 1;

void setup() {
  Serial.begin(9600); // initialize the serial port
  
  AFMS.begin();  // create with the default frequency 1.6KHz
  myMotor->setSpeed(50);  // Set RPM of stepper motor 
  
  pixels.begin(); // This initializes the NeoPixel library.

  dialSequence(); //Do a single dial sequencing on startup
}

void setPixels(uint32_t color) {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, color);
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void dialSymbol(int chevron, int led, bool last) {
   int numStep = 0;
   if (chevron == 1) {
      numStep = (SGAddress[(chevron-1)] - 1) * STEPPERSYM;
   } else {
      if (SGAddress[(chevron-1)] > SGAddress[(chevron-2)]){
        numStep = ((SGAddress[(chevron-2)] - 1) + (40 - SGAddress[(chevron-1)])) * STEPPERSYM;
      } else{
        numStep = (SGAddress[(chevron-1)] - SGAddress[(chevron-2)]) * STEPPERSYM;
      }
   }

   if (lastDir == 1) {
      myMotor->step(numStep, BACKWARD, SINGLE);
   } else {
      myMotor->step(numStep, FORWARD, SINGLE);
   }
   pixels.setPixelColor(4, orange);
   pixels.show();
   delay(BlinkDelay);
   if (!last) {
      pixels.setPixelColor(4, off);
   } else {
      pixels.setPixelColor(0, orange);
   }
   pixels.setPixelColor(led, orange);
   pixels.show();
}

void resetDial() {
  setPixels(off);
  lastDir = 0;
}

void dialSequence() {
  setPixels(off);
  int led = 5;
  for (int i=1;i<=7;i++) {
    if (led > 7)
      led = 1;
    dialSymbol(i, led, (i>=7));
    led++;
  }
}

void generateSequence() {
   bool looking = true; 
   for (int i=0;i<7;i++) {
      looking = true;
      int found = 0;
      while (looking) {
         int ran = random(1,39);
         bool fail = false;
         for (int c=0;c<7;c++) {
             if (SGAddress[c]) {
                fail = true;
             }
         }
         if (fail == false)
            looking = false;
      }
      SGAddress[i] = found;
   }
   Serial.println("Sequence Generated");
}

void programSequence() {
  
}

void dumpSequence() {
  
}

void parseCommand() {
  char cmd = Serial.read();
  switch (cmd) {
    case 'D':
      dialSequence();
      break;
    case 'G':
      generateSequence();
      break;
    case 'P':
      programSequence();
      break;
    case 'S':
      dumpSequence();
      break;
  }
}

void loop() {
  if (Serial.available())
    parseCommand();
}
