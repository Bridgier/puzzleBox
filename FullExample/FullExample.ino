
#include <Wire.h>
#include <Adafruit_NeoPixel.h>     
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
         
#define PIN 6 
         
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);
 /* Assign a unique ID to this sensor at the same time */

Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345); 

sensors_event_t event;  
float heading;
float myBearing;

int potPin = 5;
int intensity = 65;
int openPin = 7;
int currentOpenState = 1;  // 1 == LOCKED
int step;
int indicator;
int north;
int mbo;

int maxInt = 100;
int minInt = 0;
int currentLED = 0;
int formerN = 0;
int formerI = 0;

int heartbeat = 0;


void setup()
{
  Serial.begin(9600);  
  pinMode(openPin, INPUT);
  digitalWrite(openPin, HIGH);

  //Serial.println("locking box");
  Wire.begin();
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(currentOpenState);              // sends one byte  
  Wire.endTransmission();    // stop transmitting
  //Serial.println("message sent");
 
  Serial.println("Magnetometer Test"); Serial.println("");
    /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
        Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
        while(1);
  }
  
  strip.begin();
  strip.show();
}

  union received_ulong {
    byte b[4];
    unsigned long val;
  };

  union received_double {
    byte b[4];
    double val;
  };

   union received_ulong range;
   union received_double course;    
   byte data_available;

void loop()
{
   int val = digitalRead(openPin);
   
   if((val == LOW) && (currentOpenState == 1)) {
      currentOpenState = 0;
      Wire.begin();
      Wire.beginTransmission(4); // transmit to device #4
      Wire.write(currentOpenState);              // sends one byte  
      Wire.endTransmission();    // stop transmitting     
   }
   else if ((val == HIGH) && (currentOpenState == 0)) {
     currentOpenState = 1;
     Wire.begin();
     Wire.beginTransmission(4); // transmit to device #4
     Wire.write(currentOpenState);              // sends one byte  
     Wire.endTransmission();    // stop transmitting     
   }
   
    mag.getEvent(&event);
    
    heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / M_PI;
        
    if (heading < 0)
    {
      heading = 360 + heading;
    }  
  
    indicator = (24 - (heading/15));

    north = indicator;
    
    Wire.beginTransmission(4);
    
    Wire.write(2);  // request the gps info
    
    delay(10);
    
    Wire.requestFrom(4,9);
 
    int ptr = 0;
  
    while(Wire.available())    // slave may send less than requested
    {
        byte b = Wire.read();
       
        if(ptr == 0)
          data_available = b;
        else if(ptr < 5)
          range.b[ptr - 1] = b;
        else
          course.b[ptr - 5] = b;
    
        ptr++;  
      }
      
    Wire.endTransmission();
  
    Serial.print("DA:");
    Serial.print(data_available);
  
    if(data_available) {
      
        Serial.print("     ");
        Serial.print(range.val);
        Serial.print("     ");
        Serial.print(course.val);    
        
        mbo = (course.val/15);
        north = indicator;

        if(indicator + mbo > 23)
          indicator = mbo - (23 - indicator);
        else 
          indicator = indicator + mbo;

        if(range.val < 15) {
                
         //UNLOCK THE BOX!!!!
           Wire.beginTransmission(4); // transmit to device #4
           Wire.write(0);              // sends one byte  
           Wire.endTransmission();    // stop transmitting
 
           theaterChaseRainbow(50);     
           
           // If they haven't opened the box yet, lock it.
           Wire.beginTransmission(4); // transmit to device #4
           Wire.write(1);              // sends one byte  
           Wire.endTransmission();    // stop transmitting           
       }
       else {
         setLED(indicator,north);
         //setColors(strip.Color(0, intensity, 0), 50, indicator, north); // Green
       }
     
     } else {
       setRing(strip.Color(intensity,0,0), north);
   
       delay(500);
   
       setRing(0,north);

       delay(500);
     }
     Serial.println();
     //delay(1000);
}


void setRing(uint32_t c, uint8_t north) {
  
   for(uint16_t i=0; i< strip.numPixels(); i++) {
     if(i == north)
       strip.setPixelColor(i, strip.Color(0,0,intensity));
     else 
       strip.setPixelColor(i, c);
   }
   strip.show(); 
}

void setLED(uint8_t indicator, uint8_t north) {
  if(currentLED == 24)
  {
    currentLED = 0;
  }

  //if(formerI != indicator) {
      strip.setPixelColor(formerI, 0);
  //}
  
  if(formerN != north) {
    strip.setPixelColor(formerN, 0);
  }
  
  //strip.setPixelColor(currentLED, strip.Color(0,128,0));
  
  for(int i=0;i<8;i++) {
    if(currentLED - i < 0)
    {
      strip.setPixelColor(currentLED -i + 24, strip.Color(0,112 - (16*i),0));
    }
    else {
      strip.setPixelColor(currentLED -i, strip.Color(0,112 - (16*i), 0));
    }
  }  
  
    int skips;

    if(range.val >= 2000)
      skips = 30;
    else if((range.val < 2000) && (range.val >=10))
      skips = map(range.val, 10, 2000, 2, 30);
    else
      skips = 2;
      
    /*else if((range.val < 2000) && (range.val >= 1000)) 
      skips = 20;
    else if((range.val < 1000) && (range.val >= 500))
      skips = 10;
    else if((range.val < 500) && (range.val >= 100))
      skips = 5;
    else if((range.val < 100))
      skips = 2;*/
  //skips = 2;
  if(heartbeat % skips == 0) {
    strip.setPixelColor(indicator, strip.Color(128, 0, 0)); // - (skips * 4), skips * 2,skips * 4));
  }


  strip.setPixelColor(north, strip.Color(0,0,128));
  

  
  delay(50);

  strip.show();
  currentLED++;
  formerI = indicator;
  formerN = north;
  heartbeat++;

  if(heartbeat == 10000)
    heartbeat = 0;
}


void setColors(uint32_t c, uint8_t wait, uint8_t indicator, uint8_t north) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    
    if(indicator == i)
      strip.setPixelColor(i, strip.Color(intensity, 0, 0));
    else if(north == i)
      strip.setPixelColor(i, strip.Color(0, 0, intensity));
    else 
      strip.setPixelColor(i, c);
      
    strip.setPixelColor(i == 0 ? 23 : i-1,0);
    strip.show();
    
    if(range.val >= 2000)
      delay(500);
    else if((range.val < 2000) && (range.val >= 1000)) 
      delay(300);
    else if((range.val < 1000) && (range.val >= 500))
      delay(200);
    else if((range.val < 500) && (range.val >= 100))
      delay(100);
    else if((range.val < 100))
      delay(range.val);
  }  
}

 
    
void setColorsb(uint32_t c, uint8_t wait, uint8_t indicator, uint8_t north) {

  int read = 0;
  if(intensity >= maxInt)
    {
      intensity = minInt;
      read = 1;
    }
//  else if(intensity == 30) {
//      step = 5;
//    }

//  intensity += 1; //step;
  
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, intensity, 0));
  }
  strip.setPixelColor(north, strip.Color(0, 0, intensity));    
  strip.setPixelColor(indicator, strip.Color(intensity, 0, 0));
  strip.show();
  delay(1);
   
  intensity += 1;
  
//  smartDelay(100);
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

