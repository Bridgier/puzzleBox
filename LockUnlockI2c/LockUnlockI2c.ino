// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.


#include <TinyGPS++.h>
//#include <SoftwareSerial.h>

#include <Servo.h> 
#include <Wire.h>
 

static const int RXPin = 3, TXPin = 4;
static const int LCDRXP = 8, LCDTXP = 9;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
//SoftwareSerial ss(RXPin, TXPin);
 
Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
 
int pos = 0;    // variable to store the servo position 
int pin = 13;

int servoLock = 100;
int servoOpen = 15;

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial.available()) {
      byte b = Serial.read();
      gps.encode(b);
      //Serial.write(b);
    }
  } while (millis() - start < ms);
}

union send_ulong {
        byte b[4];
        unsigned long val;
};

union send_double {
  byte b[4];
  double val;
};

union send_ulong range;
union send_double heading;
byte send[9];

void setup() 
{ 
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  pinMode(pin,OUTPUT);
 
  Wire.begin(4);
 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(slaveRequest);
  
  Serial.begin(9600);
  //ss.begin(GPSBaud);

  //Serial.println("Beginning");
} 

int eventCount;

static const double LONDON_LAT = 43.550032, LONDON_LON = -116.128083;
//rose garden static const double LONDON_LAT = 43.609265, LONDON_LON = -116.205102;

// reuseum static const double LONDON_LAT = 43.620804, LONDON_LON = -116.239803;
int transmitGPS = 0;

void loop() 
{
  smartDelay(0);
       
  range.val =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON);
  
  heading.val =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON);
      
   smartDelay(0);
} 

void slaveRequest() {
 
  if(transmitGPS) {
      transmitGPS = 0;
      
      int i;
  
      send[0] = gps.location.isValid() ? 1 : 0;
  
      for(i=0;i<4;i++) {
         send[i+1] = range.b[i]; 
      }
      
      for(i=0;i<4;i++) {
        send[i+5] = heading.b[i];
      }
  
      Wire.write(send,9);
/*      Serial.print(send[0]);
      Serial.print(" ");
      Serial.print(range.val);
      Serial.print(" ");
      Serial.println(heading.val); */
  }

}
void receiveEvent(int howMany)
{    
  byte x = Wire.read();    // receive byte as an integer
  
/*  Serial.print("Got: ");
  Serial.println(x); */
  
  if(x == 0) {
    myservo.write(servoOpen);
  }
  else if(x == 1) {
    myservo.write(servoLock);    
  }
  else if (x == 2) {    
     transmitGPS = 1;
  }
  eventCount++;
}


