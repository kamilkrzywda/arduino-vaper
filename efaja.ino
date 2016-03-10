#include <EEPROM.h>

//coil resistance and batery voltage sensors
const int coilSensor = A0;
const int batterySensor = A1;

//buttons
const int button1 = A2;
const int button2 = A3;

//heater button and mosfet output
const int heaterButton = 9;
const int heaterOutput = 10;

//led pins
const int led1 = 3;
const int led2 = 5;
const int led3 = 6;

//buttons
int button1Status = 0;
int button2Status = 0;
int heaterButtonStatus = 0;

//sensors
int coilSensorValue = 0;
int batterySensorValue = 0;


//power calculations
float coilOhms = 1.00;
float batteryVoltage = 8.00;

float powerMultiplier = 2;
int powerSet = 0;
float powerMax = 20;
float powerPWM = 0;

//heating
int heating = 0;

//sleep
int sleep = 0;

/*
 * increase power setting
 */
void addPower(int setPower){
  powerSet+=setPower;
  if(powerSet > 31){
    powerSet = 31;
  }
  EEPROM.write(0, powerSet);
  setLeds();
}

/*
 * decrease power setting
 */
void subPower(int setPower){
  powerSet-=setPower;
  if(powerSet < 1){
    powerSet = 1;
  }
  EEPROM.write(0, powerSet);
  setLeds();
}

/*
 * update status leds
 */
void setLeds(){
  if(powerSet <= 10){
    analogWrite(led1, powerSet*powerSet*2);
    analogWrite(led2,0);
    analogWrite(led3,0);
  }else if(powerSet <= 20){
    analogWrite(led1,255);
    analogWrite(led2, (powerSet-10)*(powerSet-10)*2);
    analogWrite(led3,0);
  }else if(powerSet <= 30){
    analogWrite(led1,255);
    analogWrite(led2,255);
    analogWrite(led3, (powerSet-20)*(powerSet-20)*2);
  }else{
    analogWrite(led1,255);
    analogWrite(led2,0);
    analogWrite(led3,255);
  }
}

/*
 * read button
 */
void readStatus(){
  button1Status = analogRead(button1);
  button2Status = analogRead(button2);
  heaterButtonStatus = digitalRead(heaterButton);

  if(heaterButtonStatus == HIGH){
    sleep = 0;
    setLeds();
  }
  
  if(heating == 0){
    coilSensorValue = analogRead(coilSensor);
    batterySensorValue = analogRead(batterySensor);
    
    batteryVoltage = ((float)batterySensorValue)/18;
    coilOhms = (float)(coilSensorValue)/100;
    powerMax = batteryVoltage*batteryVoltage/coilOhms;
    powerPWM = powerSet/powerMax;
    if(powerPWM > 1){
      powerPWM = 1;
    }
    if(powerSet == 31){
      powerPWM = 1;
    }
    
    //serialDebug();
  }
}

/*
 * buttons actions
 */
void checkButtons(){
  if(button1Status < 500){
    sleep = 0;
    addPower(1);
    delay(50);
  }
  
  if(button2Status < 500){
    sleep = 0;
    subPower(1);
    delay(50);
  }
  
  if(heaterButtonStatus){
    heating += 1;
  }else{
    heating = 0;
  }
}

/*
 * heating
 */
void heat(){
  if(heating > 0){
    if(batteryVoltage > 7){
      analogWrite(heaterOutput,255-255*powerPWM);
      if(heating > 100){
        heating = 0;
        digitalWrite(heaterOutput,HIGH);
      }
    }else{
      for(int i=0;i<6;i++){
        analogWrite(led1,255);
        analogWrite(led2,255);
        analogWrite(led3,255);
        delay(500);
        analogWrite(led1,0);
        analogWrite(led2,0);
        analogWrite(led3,0);
        delay(500);
      }
    }
  }else{
    digitalWrite(heaterOutput,HIGH);
  }
}

/*
 * debug
 */
void serialDebug(){
  Serial.print(button1Status);
  Serial.print("\t");
  Serial.print(button2Status);
  Serial.print("\t");
  Serial.print(heating);
  Serial.print("\t");
  Serial.print("coil ohms: ");
  Serial.print(coilOhms);
  Serial.print("\t");
  Serial.print(coilSensorValue);
  Serial.print("\t");
  Serial.print("battery volatege: ");
  Serial.print(batteryVoltage);
  Serial.print("\t");
  Serial.print("max power: ");
  Serial.print(powerMax);
  Serial.print("\t");
  Serial.print("power pwm: ");
  Serial.print(powerPWM);
  Serial.println("");
}

void setup(){
  Serial.begin(115200);
  addPower(EEPROM.read(0));
  digitalWrite(heaterOutput,HIGH);
}

void loop() {
  sleep++;
  
 readStatus();

  if(sleep == 20000){
    analogWrite(led1,0);
    analogWrite(led2,0);
    analogWrite(led3,0);
  }else if(sleep < 20000){
    checkButtons();
    heat();
  }
  
  delay(1);
  
}
