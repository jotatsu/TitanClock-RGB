//RTC libraries 
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include "DS3231.h"
//WS2812B libraries

#ifdef __AVR__
  #include <avr/power.h>
#endif

/**************RGB strip Definitions**************/

// Define pins for led strip
#define PIN            9
#define PINBLINK       6

// Define pixels in the strip
#define NUMPIXELS      56
#define NUMPIXELS3     2

//Setup led strips
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel blinks  = Adafruit_NeoPixel(NUMPIXELS3, PINBLINK, NEO_GRB + NEO_KHZ800);


/**************RTC Definitions**************/
RTClib rtc;

/**************Bluetooth Definitions**************/
SoftwareSerial BTserial(2, 3); // RX | TX


/*******Control states for the clock********/
int delayval = 1000; 

int hourDigit1=0;
int hourDigit2=0;
int miniteDigit1=0;
int miniteDigit2=0;

int blinkState=0;
/////*Globals*/////

#define SEGMENT_SIZE    14
#define TOTAL_SEGMENTS  4


/////*Number templates*/////
//This define a template for number inside a segment according to the wiring of the project, 
//1 represent the led is on and should be colored
//0 represent that is should be off
//2 leds per segment, 14 leds per number

//Mask for right to left numbers 
unsigned int rgbColourInv[11][14]={
  {1,1,1,0,1,1,1,1,1,1,0,1,1,1}, //0
  {0,1,1,0,1,1,0,0,0,0,0,0,0,0}, //1
  {1,0,0,1,1,1,1,1,0,0,1,1,1,1}, //2
  {1,1,1,1,1,1,1,1,0,0,1,0,0,1}, //3
  {0,1,1,1,1,1,0,0,1,1,1,0,0,0}, //4
  {1,1,1,1,0,0,1,1,1,1,1,0,0,1}, //5
  {1,1,1,1,0,0,1,1,1,1,1,1,1,1}, //6
  {0,1,1,0,1,1,1,1,0,0,0,0,0,0}, //7
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1}, //8
  {0,1,1,1,1,1,1,1,1,1,0,0,0,0}, //9
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //nill
};

//Mask for left to right numbers 
unsigned int rgbColour[11][14]={
  {1,1,1,0,1,1,1,1,1,1,0,1,1,1}, //0
  {0,0,0,0,0,0,0,0,1,1,0,1,1,0}, //1
  {1,1,1,1,0,0,1,1,1,1,1,0,0,1}, //2
  {1,0,0,1,0,0,1,1,1,1,1,1,1,1}, //3
  {0,0,0,1,1,1,0,0,1,1,1,1,1,0}, //4
  {1,0,0,1,1,1,1,1,0,0,1,1,1,1}, //5
  {1,1,1,1,1,1,1,1,0,0,1,1,1,1}, //6
  {0,0,0,0,0,0,1,1,1,1,0,1,1,0}, //7
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1}, //8
  {0,0,0,0,1,1,1,1,1,1,1,1,1,0}, //9
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //nill
};

//Creates a RGB color schedule by hourR
unsigned int colorSchedule[24][3]= {
  {40,0,0},  //00
  {40,0,0},  //01
  {40,0,0},  //02
  {40,0,0},  //03
  {40,0,0},  //04
  {40,0,0},  //05   
  {180,0,0},  //06
  {180,0,0},  //07
  {160,20,0},  //08
  {140,40,0},  //09
  {120,60,0},  //10
  {100,80,0},  //11
  {80,100,0},  //12
  {60,120,0},  //13
  {40,140,0},  //14
  {20,160,0},  //15
  {0,180,0},  //16
  {0,160,20},  //17
  {0,140,40},  //18
  {0,120,60},  //19
  {0,100,80},  //20
  {0,80,100},  //21
  {40,0,0},  //22
  {40,0,0},  //23
}; 
int currColor[3]= {0,0,160}; 

/////*-------*/////
void setup() {

  pixels.begin(); // This initializes the NeoPixel library.
  blinks.begin(); // This initializes the NeoPixel library.
  Serial.begin(9600);
  BTserial.begin(9600);
  delay(1000); 
  Wire.begin(); 
}

//Set the number's color for a given segment
void setNumber(int number, int segment,  int r,int g,int b)
{
  //Segment are offsets of 14
  int offset = SEGMENT_SIZE * segment;
  
  if(segment < TOTAL_SEGMENTS)
  {
    for (int i = 0 ; i < SEGMENT_SIZE; i++)
    {
      if( rgbColour[number][i] == 0)
      {
         pixels.setPixelColor(i + offset, pixels.Color(0,0,0));  
      }else{
         pixels.setPixelColor(i + offset, pixels.Color(r,g,b));  
      }
    }  
  }
}

//Set the blinking dots color
void setBlink(int r,int g,int b)
{
      blinks.setPixelColor(0, pixels.Color(r,g,b));  
      blinks.setPixelColor(1, pixels.Color(r,g,b));  
}
 
void loop() { 

  DateTime now = rtc.now();

  hourDigit1=now.hour()/10;
  hourDigit2=now.hour()%10;
  miniteDigit1=now.minute()/10;
  miniteDigit2=now.minute()%10;    

  //Serial.println(now.hour()+':'+now.minute()+':'+now.second());
  
  //Color calculation
  currColor[0] = colorSchedule[now.hour()][0];
  currColor[1] = colorSchedule[now.hour()][1];
  currColor[2] = colorSchedule[now.hour()][2];

  //Extra power saving by disabling the first 0 in hours like 09:30
  if (hourDigit1 == 0)
  {
    setNumber(10,0,currColor[0],currColor[1],currColor[2]);
  }else{
    setNumber(hourDigit1,0,currColor[0],currColor[1],currColor[2]);
  }
  setNumber(hourDigit2,1,currColor[0],currColor[1],currColor[2]);
  setNumber(miniteDigit1,2,currColor[0],currColor[1],currColor[2]);
  setNumber(miniteDigit2,3,currColor[0],currColor[1],currColor[2]);
  if(blinkState==0)
  {
    setBlink(currColor[0],currColor[1],currColor[2]);
    blinkState = 1;
  }else
  {
    setBlink(0,0,0);
    blinkState = 0;
  }
  
  pixels.show();
  blinks.show();

  delay(delayval); 
}
