#include <Servo.h> // to control ESC
#include "HX711.h" // to interface with load cell

// Arduino Wiring
const int ESC_pin = 9;
const int LED_pin = 2; // LED used for warnings and updates
const int SDA_pin = 4;
const int SCL_pin = 5;

// Load Cell Calibration Constants
long calibration_reading = 55660; // run calibration function to update this value
float calibration_weight = 120.1; // known weight used during calibration

HX711 loadCell;
Servo esc;

void setup(){
  Serial.begin(115200);
  blinkLED(5);
  Serial.println("Welcome to the Thrust Test Stand Program");
  Serial.println("----------------------------------------\n");
  Serial.println("Choose an option to continue:");
  Serial.flush();
  Serial.println("A: Calibrate Load Cell");
  Serial.println("B: Run Thrust Test");
  while(Serial.available()==0){
  }
  char option = Serial.read();
  switch (option){
    case 'a':
      calibrateLoadCell();
      setupLoadCell();
    case 'b':
      setupLoadCell();
  }
  Serial.println("----------------------------------------\n");
  Serial.println("Load Cell Setup Complete, check for zero reading");
  Serial.print("Zeroed Load Cell Reading: ");
  Serial.print(loadCell.get_units(10),3); // average of 10 readings, rounded to 3 decimals
  Serial.print(" grams");

  Serial.println("----------------------------------------\n");
  blinkLED(3);
  Serial.flush();
  Serial.println("Please Turn on the battery-ESC switch within 5s.");
  blinkLED(5);
  armESC();
  Serial.println("----------------------------------------\n");
  Serial.println("Q for throttle up, W for throttle down, E for emergency stop");
  Serial.flush();
  blinkLED(5);
}

char command;
int throttle = 0; // percent

void loop(){
  if (Serial.available() > 0) {
    command = Serial.read();

    if(command=='e'){
      throttle = 0;
      esc.writeMicroseconds(1000);
    }
    if(command=='q'){
      //throttle += 10;
      if(throttle!=0){
        for(int i=0;i<10;i++){
          throttle +=1;
          esc.writeMicroseconds(throttle*10+1000);
        }
      }
      else{
        throttle = 10;
        esc.writeMicroseconds(1100);
      }
    }
    if(command=='w'){
      //throttle -= 10;
      for(int i=0;i<10;i++){
        throttle -=1;
        esc.writeMicroseconds(throttle*10+1000);
      }
    }
    //esc.writeMicroseconds(throttle*10+1000); // microseconds range from 1000-2000
  }
  
  Serial.print("Throttle: ");
  Serial.print(throttle);
  Serial.print("% \t | \t ");
  Serial.print("Thrust: ");
  Serial.print(loadCell.get_units(5),3);
  Serial.println(" grams");
  Serial.flush();
  digitalWrite(LED_pin,HIGH);
  delay(100);
  digitalWrite(LED_pin,LOW);
  delay(100);

}

// ESC arming function
void armESC(){
  esc.attach(ESC_pin);
  Serial.println("Starting ESC Arming Sequence...");
  long now = millis();
  while(millis()<now+5000){
    esc.writeMicroseconds(1000);
    delay(200);
  }
  esc.writeMicroseconds(1000);
  Serial.println("ESC Arming Complete!");
  blinkLED(3);
}

// Load Cell functions
void setupLoadCell(){
  loadCell.begin(SDA_pin,SCL_pin);
  loadCell.set_scale(calibration_reading/calibration_weight);
  loadCell.tare();
}

void calibrateLoadCell(){
  Serial.println("Enter the calibration weight (grams)");
  blinkLED(2);
  while(Serial.available()==0){
  }
  calibration_weight = Serial.parseFloat(SKIP_ALL, '\n');

  loadCell.begin(SDA_pin,SCL_pin);
  if (loadCell.is_ready()){
    loadCell.set_scale();
    Serial.println("Tare... remove any weights from the loadCell.");
    blinkLED(5);
    loadCell.tare();
    Serial.println("Tare done...");
    Serial.print("Place the calibration weight on the loadCell...");
    blinkLED(5);
    calibration_reading = loadCell.get_units(10);
    Serial.print("Calibration Reading: ");
    Serial.println(calibration_reading);
  }
  else{
    Serial.println("HX711 not found. Check for wiring issues.\n Close program in 10s.");
    blinkLED(10);
  }
}


// LED blinking function
void blinkLED(int seconds){
  int i = 0;
  // 1s long flashing sequence
  while(i<seconds){
    digitalWrite(LED_pin,HIGH);
    delay(500);
    digitalWrite(LED_pin,LOW);
    delay(500);
    i+=1;
  }
}
