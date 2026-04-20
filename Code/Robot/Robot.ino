#include <ESP32Servo.h>     // ESP32 Servo library from libraries manager

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;
Servo servo7;
Servo servo8;

#include <esp_now.h>
#include <WiFi.h>

int pot1;       // receiving remote joystick
int pot2;       // receiving remote joystick

// output for servos

float servo1Output = 1500;    // linear track
float servo2Output = 1500;    // rotary track

float servo3Output = 2000;    // back left leg        **Higher** number makes legs longer
float servo4Output = 1000;    // back right leg       **Lower** number makes legs longer
float servo5Output = 2000;    // middle left leg      **Higher** number makes legs longer
float servo6Output = 1000;    // middle right leg     **Lower** number makes legs longer
float servo7Output = 2000;    // front left leg       **Higher** number makes legs longer
float servo8Output = 1000;    // front right leg      **Lower** number makes legs longer

float servo1OutputFiltered = servo1Output;        // motion filtered values
float servo2OutputFiltered = servo2Output;
float servo3OutputFiltered = servo3Output;
float servo4OutputFiltered = servo4Output;
float servo5OutputFiltered = servo5Output;
float servo6OutputFiltered = servo6Output;
float servo7OutputFiltered = servo7Output;
float servo8OutputFiltered = servo8Output;

// offset for servos - start with zero and adjust until all servos are in the correct starting position with all legs down

int servo1Offset = 0;     // linear track 
int servo2Offset = 0;     // rotary track
int servo3Offset = 0;   // back left leg
int servo4Offset = 0;   // back right leg
int servo5Offset = 0;   // middle left leg
int servo6Offset = 0;     // middle right leg
int servo7Offset = 0;     // front left leg
int servo8Offset = 0;  // front right leg

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int a;    // pot1
  int b;    // pot2

} struct_message;

unsigned long currentMillis;
unsigned long previousMillis = 0;   // set up timers
long previousWalkMillis = 0;        // set up timers

int stepTime = 250;
int legScaler = 0;

int walkCount = 0;
int walkAction = 0;

int legUp1 = 1500;
int legDown1 = 2000;
int legUp2 = 1500;
int legDown2 = 1000;

int filterVal;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  pot1 = myData.b - 1975;     // modify offset to get zero when stick is stationary
  pot2 = myData.a - 1985;     // modify offset to get zero when stick is stationary

  // un-comment lines below to get centre stick positions

/*
  Serial.print(myData.a);
  Serial.print(" , ");
  Serial.print(myData.b);
  Serial.print(" , ");
*/

  // threshold value for control sticks
  // makes a dead-band of +/- 50
  if (pot1 > 50) {
    pot1 = pot1 -50;
  }
  else if (pot1 < -50) {
    pot1 = pot1 +50;
  }
  else {
    pot1 = 0;
  }
  // threshold value for control sticks
  if (pot2 > 50) {
    pot2 = pot2 -50;
  }
  else if (pot2 < -50) {
    pot2 = pot2 +50;
  }
  else {
    pot2 = 0;
  }  
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  // attach servos to pins
  servo1.attach(13);      // linear track
  servo2.attach(12);      // rotary track
  servo3.attach(14);      // back left leg
  servo4.attach(27);      // back right leg
  servo5.attach(26);      // middle left leg
  servo6.attach(25);      // middle right leg
  servo7.attach(33);      // front left leg
  servo8.attach(32);      // front right leg
}
 
void loop() {

  currentMillis = millis();
  if (currentMillis - previousMillis >= 10) {     // this loop runs every 10ms
      previousMillis = currentMillis;

      if (pot1 != 0 && pot2 == 0) {
        // walk forward/backward
        walkAction = 1;
        pot1 = pot1;
        pot2 = 0;
      }
      else if (pot2 != 0 && pot1 == 0) {
        // walk rotate
        walkAction = 1;
        pot2 = pot2;
        pot1 = 0;
      } 
      else {
        walkAction = 0;
        pot1 = 0;
        pot2 = 0;
      }

      // scale  step length and timings depending on joystick values

      if (abs(pot1) > abs(pot2)) {
        stepTime = 600-(abs(pot1)/10);
        legScaler = 1400-(abs(pot1)/3);
        filterVal = 20;
      }
      else if (abs(pot2) > abs(pot1)) {
        stepTime = 800-(abs(pot2)/5);
        legScaler = 1250-(abs(pot2)/3);
        filterVal = 20;
      }
      
      legUp1 = 2000 - legScaler;
      legDown1 = 2000;
      legUp2 = 1000 + legScaler;
      legDown2 = 1000; 

      // *** START OF WALK ***
      
        if (walkAction == 1 && walkCount == 0 && currentMillis - previousWalkMillis >= stepTime) {
          // pick up first three legs
          servo3Output = legUp1;
          servo6Output = legUp2;
          servo7Output = legUp1;
          previousWalkMillis = currentMillis;
          walkCount = 1;
        }
        else if (walkCount == 1 && currentMillis - previousWalkMillis >= stepTime) {
          // slide or rotate
          servo1Output = 1500 + pot1;
          servo2Output = 1500 + pot2/3;
          previousWalkMillis = currentMillis;
          walkCount = 2;
        }
        else if (walkCount == 2 && currentMillis - previousWalkMillis >= stepTime) {
          // put down first three legs
          servo3Output = legDown1;
          servo6Output = legDown2;
          servo7Output = legDown1;
          previousWalkMillis = currentMillis;
          walkCount = 3;
        }
        else if (walkCount == 3 && currentMillis - previousWalkMillis >= stepTime) {
          // pick up second three legs
          servo4Output = legUp2;
          servo5Output = legUp1;
          servo8Output = legUp2;  
          previousWalkMillis = currentMillis;
          walkCount = 4;
        }
        else if (walkCount == 4 && currentMillis - previousWalkMillis >= stepTime) {
          // slide or rotate
          servo1Output = 1500 - pot1;
          servo2Output = 1500 - pot2/3;
          previousWalkMillis = currentMillis;
          walkCount = 5;
        }
        else if (walkCount == 5 && currentMillis - previousWalkMillis >= stepTime) {
          // put down second three legs
          servo4Output = legDown2;
          servo5Output = legDown1;
          servo8Output = legDown2;
          previousWalkMillis = currentMillis;
          walkCount = 0;
        } 

      // constrain linear and rotation axis so the legs don't crash into each other
      servo1Output = constrain(servo1Output,700,2300);     // linear track
      servo2Output = constrain(servo2Output,1000,2000);     // rotary track

      // filter values for smoother motion
      servo1OutputFiltered = filter(servo1Output, servo1OutputFiltered, filterVal);
      servo2OutputFiltered = filter(servo2Output, servo2OutputFiltered, filterVal);
      servo3OutputFiltered = filter(servo3Output, servo3OutputFiltered, filterVal);
      servo4OutputFiltered = filter(servo4Output, servo4OutputFiltered, filterVal);
      servo5OutputFiltered = filter(servo5Output, servo5OutputFiltered, filterVal);
      servo6OutputFiltered = filter(servo6Output, servo6OutputFiltered, filterVal);
      servo7OutputFiltered = filter(servo7Output, servo7OutputFiltered, filterVal);
      servo8OutputFiltered = filter(servo8Output, servo8OutputFiltered, filterVal);      

      // write to servos
      servo1.writeMicroseconds(servo1OutputFiltered + servo1Offset);     // linear track
      servo2.writeMicroseconds(servo2OutputFiltered + servo2Offset);     // rotary track
      servo3.writeMicroseconds(servo3OutputFiltered + servo3Offset);     // back left leg
      servo4.writeMicroseconds(servo4OutputFiltered + servo4Offset);     // back right leg
      servo5.writeMicroseconds(servo5OutputFiltered + servo5Offset);     // middle left leg
      servo6.writeMicroseconds(servo6OutputFiltered + servo6Offset);     // middle right leg
      servo7.writeMicroseconds(servo7OutputFiltered + servo7Offset);     // front left leg
      servo8.writeMicroseconds(servo8OutputFiltered + servo8Offset);     // front right leg

    }  // end of 10ms timed loop
    
} // end of main loop


// motion filter to filter motions
float filter(float prevValue, float currentValue, int filter) {  
  float lengthFiltered =  (prevValue + (currentValue * filter)) / (filter + 1);  
  return lengthFiltered;  
}
