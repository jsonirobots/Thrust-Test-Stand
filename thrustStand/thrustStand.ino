#include <Servo.h> // to control ESC
#include "HX711.h" // to interface with load cell

// Arduino Wiring
const int ESC_pin = 9;
const int LED_R_pin = 2; // LEDs used for warnings and updates
const int LED_L_pin = 3; // LEDs used for warnings and updates
const int SDA_pin = 4;
const int SCL_pin = 5;

//bool ledL = false;
//bool ledR = false;

const int motor_low_signal = 1000; // microseconds
const int motor_high_signal = 2000; //microseconds
const float throttle_inc = (motor_high_signal-motor_low_signal)/100; //the increment for each 1% change

// Load Cell Calibration Constants
long calibration_reading = 55660; // run calibration function to update this value
float calibration_weight = 120.1; // known weight used during calibration

HX711 loadCell;
Servo esc;

void setup(){
  Serial.begin(115200);
  pinMode(LED_L_pin,OUTPUT);
  pinMode(LED_R_pin,OUTPUT);
  blinkLED('l',2,1);
  LEDon('l',true);
  Serial.println("Welcome to the Thrust Test Stand Program");
  Serial.println("----------------------------------------\n");
  Serial.println("Choose an option to continue:");
  Serial.flush();
  Serial.println("A: Calibrate Load Cell");
  Serial.println("B: Run Manual Thrust Test");
  Serial.println("C: Run Auto Thrust Test");
  while(Serial.available()==0){
  }
  char option = Serial.read();
  LEDon('r',true);
  switch (option){
    case 'a':
      calibrateLoadCell();
      setupLoadCell();
      break;
    case 'b':
      setupLoadCell();
      armESC();
      manualTestInstructions();
      manualTest();
      break;
    case 'c':
      setupLoadCell();
      armESC();
      autoTestInstructions();
      autoTest();
      break;
  }
//  LEDon('r',false);
//  Serial.println("----------------------------------------\n");
//  Serial.println("Load Cell Setup Complete, check for zero reading");
//  Serial.print("Zeroed Load Cell Reading: ");
//  Serial.print(loadCell.get_units(10),3); // average of 10 readings, rounded to 3 decimals
//  Serial.print(" grams");
//  Serial.println("----------------------------------------\n");
//
//  blinkLED('r',2,1);
//  LEDon('r',true);
//  LEDon('l',true);
//  Serial.flush();
//  Serial.println("Please Turn on the battery-ESC switch within 5s.");
//  blinkLED('r',5,1);
//  LEDon('l',true);
  //armESC();
//  Serial.println("----------------------------------------\n");
//  Serial.println("Q for throttle up, W for throttle down, E for emergency stop");
//  Serial.flush();
//  blinkLED('t',3,1);
  LEDon('t',true);
}

char userCommand;
int throttle = 0; // percent
int ledl = HIGH;
int ledr = LOW;

void loop(){
  
  
//  Serial.print("Throttle: ");
//  Serial.print(throttle);
//  Serial.print("% \t | \t ");
//  Serial.print("Thrust: ");
//  Serial.print(loadCell.get_units(5),3);
//  Serial.println(" grams");
//  Serial.flush();
//  digitalWrite(LED_L_pin,HIGH);
//  digitalWrite(LED_R_pin,LOW);
//  delay(200);
//  digitalWrite(LED_L_pin,LOW);
//  digitalWrite(LED_R_pin,HIGH);
//  delay(200);

}

void runLED(int times){
  for(int k=0;k<times;k+=1){
    digitalWrite(LED_L_pin,HIGH);
    digitalWrite(LED_R_pin,LOW);
    delay(200);
    digitalWrite(LED_L_pin,LOW);
    digitalWrite(LED_R_pin,HIGH);
    delay(200);
  }
}

void reportThrust(){
  Serial.print("Throttle: ");
  Serial.print(throttle);
  Serial.print("% \t | \t ");
  Serial.print("Thrust: ");
  Serial.print(loadCell.get_units(5),3);
  Serial.println(" grams");
  Serial.flush();
}

void flashThrustLED(int times,int wait){
  for(int k=0;k<times;k+=1){
    Serial.print("Throttle: ");
    Serial.print(throttle);
    Serial.print("% \t | \t ");
    Serial.print("Thrust: ");
    Serial.print(loadCell.get_units(5),3);
    Serial.println(" grams");
    Serial.flush();
    digitalWrite(LED_L_pin,HIGH);
    digitalWrite(LED_R_pin,LOW);
    delay(wait);
    digitalWrite(LED_L_pin,LOW);
    digitalWrite(LED_R_pin,HIGH);
    delay(wait);
  }
}

void autoTestInstructions(){
  Serial.flush();
  Serial.println("The Automatic Thrust Test will begin now....");
  Serial.println("  The test will increase throttle by 10% each time,\n  spending 4s at each throttle.");
  Serial.println("  Toggle the power switch at any point to shut off the system.\n");
  blinkLED('t',5,3);
}

void autoTest(){
  //blinkLED('t',5,3);
  // ramp up loop
  flashThrustLED(5,200);
  throttle = 10;
  while(throttle<100){
    esc.writeMicroseconds(motor_low_signal + throttle_inc*throttle);
    if(throttle%10==0){
      flashThrustLED(5,200);
    }
    else{
      flashThrustLED(1,100);
    }
    throttle+=2;
  }
  throttle = 100;
  // ramp down loop
  while(throttle>=10){
    esc.writeMicroseconds(motor_low_signal + throttle_inc*throttle);
    if(throttle%10==0){
      flashThrustLED(5,200);
    }
    else{
      flashThrustLED(1,100);
    }
    throttle-=2;
  }
  throttle = 0;
  esc.writeMicroseconds(motor_low_signal + throttle_inc*throttle);
  flashThrustLED(5,200);

  blinkLED('t',3,2);
  Serial.println("Finished Auto Test...");
}

void manualTestInstructions(){
  Serial.println("Q for throttle up, W for throttle down, E for emergency stop, X for quit.");
  Serial.flush();
  blinkLED('t',3,1);
}

void manualTest(){
  bool runTest = true;
  while(runTest){
    if (Serial.available() > 0) {
      userCommand = Serial.read();
  
      switch(userCommand){
        case 'e':
          throttle = 0;
          esc.writeMicroseconds(motor_low_signal);
          break;
        case 'q':
          //throttle += 10;
          if(throttle!=0){
            for(int i=0;i<10;i++){
              throttle +=1;
              esc.writeMicroseconds(throttle*throttle_inc+motor_low_signal);
              delay(100);
              if(i%2==0){
                digitalWrite(LED_L_pin,ledl);
                digitalWrite(LED_R_pin,ledr);
                ledl = (ledl==HIGH)?LOW:HIGH;
                ledr = (ledr==LOW)?HIGH:LOW;
              }
            }
          }
          else{
            throttle = 10;
            esc.writeMicroseconds(motor_low_signal+throttle*throttle_inc);
          }
          break;
        case 'w':
          //throttle -= 10;
          for(int i=0;i<10;i++){
            throttle -=1;
            esc.writeMicroseconds(throttle*throttle_inc+motor_low_signal);
            delay(100);
            if(i%2==0){
                digitalWrite(LED_L_pin,ledl);
                digitalWrite(LED_R_pin,ledr);
                ledl = (ledl==HIGH)?LOW:HIGH;
                ledr = (ledr==LOW)?HIGH:LOW;
              }
          }
          break;
        case 'x':
          esc.writeMicroseconds(motor_low_signal);
          runTest = false;
          break;
      }
      //esc.writeMicroseconds(throttle*10+1000); // microseconds range from 1000-2000
    }
    Serial.flush();
    flashThrustLED(1,200);
  }
}

// ESC arming function
void armESC(){
  Serial.println("Please Turn on the battery-ESC switch within 5s.");
  blinkLED('r',5,1);
  LEDon('l',true);
  esc.attach(ESC_pin,motor_low_signal,motor_high_signal);
  Serial.println("Starting ESC Arming Sequence...");
  long now = millis();
  while(millis()<now+2000){
    esc.writeMicroseconds(motor_high_signal);
    delay(200);
  }
  while(millis()<now+3000){
    esc.writeMicroseconds(motor_low_signal);
    delay(200);
  }
  esc.writeMicroseconds(motor_low_signal);
  Serial.println("ESC Arming Complete!");
  Serial.println("----------------------------------------\n");
  //blinkLED('t',3);
}

// Load Cell functions
void setupLoadCell(){
  loadCell.begin(SDA_pin,SCL_pin);
  loadCell.set_scale(calibration_reading/calibration_weight);
  loadCell.tare();
  
  LEDon('r',false);
  Serial.println("----------------------------------------\n");
  Serial.println("Load Cell Setup Complete, check for zero reading");
  Serial.print("Zeroed Load Cell Reading: ");
  Serial.print(loadCell.get_units(10),3); // average of 10 readings, rounded to 3 decimals
  Serial.print(" grams");
  Serial.println("----------------------------------------\n");
  blinkLED('r',2,1);
  LEDon('r',true);
  LEDon('l',true);
  Serial.flush();
}

void calibrateLoadCell(){
  Serial.println("Enter the calibration weight (grams)");
  blinkLED('r',2,1);
  while(Serial.available()==0){
  }
  calibration_weight = Serial.parseFloat(SKIP_ALL, '\n');

  loadCell.begin(SDA_pin,SCL_pin);
  if (loadCell.is_ready()){
    loadCell.set_scale();
    Serial.println("Tare... remove any weights from the loadCell.");
    blinkLED('r',5,1);
    loadCell.tare();
    Serial.println("Tare done...");
    Serial.print("Place the calibration weight on the loadCell...");
    blinkLED('r',5,1);
    calibration_reading = loadCell.get_units(10);
    Serial.print("Calibration Reading: ");
    Serial.println(calibration_reading);
  }
  else{
    Serial.println("HX711 not found. Check for wiring issues.\n Close program in 10s.");
    blinkLED('s',10,1);
  }
}

void LEDon(char led,bool turnon){
  switch(led){
    case 'l':
      if(turnon){
        digitalWrite(LED_L_pin,HIGH);
      }
      else{
        digitalWrite(LED_L_pin,LOW);
      }
      break;
    case 'r':
      if(turnon){
        digitalWrite(LED_R_pin,HIGH);
      }
      else{
        digitalWrite(LED_R_pin,LOW);
      }
      break;
    case 't':
      if(turnon){
        digitalWrite(LED_L_pin,HIGH);
        digitalWrite(LED_R_pin,HIGH);
      }
      else{
        digitalWrite(LED_L_pin,LOW);
        digitalWrite(LED_R_pin,LOW);
      }
      break;
  }
}

// LED blinking function
void blinkLED(char led,int seconds,int rate){
  int i = 0;
  rate = int(1000/(rate*2));
  // 1s long flashing sequence
  while(i<seconds){
    switch(led){
      case 'l':
        digitalWrite(LED_L_pin,HIGH);
        delay(rate);
        digitalWrite(LED_L_pin,LOW);
        delay(rate);
        break;
      case 'r':
        digitalWrite(LED_R_pin,HIGH);
        delay(rate);
        digitalWrite(LED_R_pin,LOW);
        delay(rate);
        break;
      case 't':
        digitalWrite(LED_L_pin,HIGH);
        digitalWrite(LED_R_pin,HIGH);
        delay(rate);
        digitalWrite(LED_L_pin,LOW);
        digitalWrite(LED_R_pin,LOW);
        delay(rate);
        break;
      case 's':
        digitalWrite(LED_L_pin,HIGH);
        digitalWrite(LED_R_pin,LOW);
        delay(rate);
        digitalWrite(LED_L_pin,LOW);
        digitalWrite(LED_R_pin,HIGH);
        delay(rate);
        break;
    }
    i+=1;
  }
  digitalWrite(LED_L_pin,LOW);
  digitalWrite(LED_R_pin,LOW);
}
