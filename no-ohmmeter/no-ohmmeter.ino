#include <EEPROM.h>

//coil resistance and batery voltage sensors
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

int led1Light = 0;
int led2Light = 0;
int led3Light = 0;

//buttons
int button1Status = 0;
int button2Status = 0;
int heaterButtonStatus = 0;

//sensors
int batterySensorValue = 0;
float batteryVoltage = 8.00;

//power
int powerSet = 0;
int powerPWM = 0;

//heating
int heating = 0;

//sleep
int sleep = 0;

/*
 * change  power setting
 */
void setPower(int power){
  powerSet = power;
  if(powerSet > 193){
    powerSet = 193;
    powerPWM = 255;
  }else if(powerSet < 1){
    powerSet = 1;
  }else{
    powerPWM = powerSet;
  }
  EEPROM.write(0, powerSet);
}

void updateLeds(){
  analogWrite(led1,led1Light);
  analogWrite(led2,led2Light);
  analogWrite(led3,led3Light);
}

void setLeds(){
  if(powerSet <= 64){
    led1Light = powerSet*powerSet/32;
    led2Light = 0;
    led3Light = 0;
  }else if(powerSet <= 128){
    led1Light = 255;
    led2Light = (powerSet-64)*(powerSet-64)/32;
    led3Light = 0;
  }else if(powerSet <= 192){
    led1Light = 255;
    led2Light = 255;
    led3Light = (powerSet-128)*(powerSet-128)/32;
  }else{
    led1Light = (sin(((float)millis()+1000)/500)+1)*100;
    led2Light = (sin(((float)millis()+500)/500)+1)*100;
    led3Light = (sin(((float)millis())/500)+1)*100;
  }
  updateLeds();
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
    batterySensorValue = analogRead(batterySensor);
    batteryVoltage = (float)batterySensorValue/37;
  }
}

/*
 * buttons actions
 */
void checkButtons(){
  if(button1Status < 900){
    sleep = 0; 
    setPower(powerSet+1);
    delay(10);
  }
  
  if(button2Status < 900){
    sleep = 0;
    setPower(powerSet-1);
    delay(10);
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
      analogWrite(heaterOutput,255-powerPWM);
      Serial.println(255-powerPWM);
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
  Serial.print("battery volatege: ");
  Serial.print(batteryVoltage);
  Serial.print("\t");
  Serial.print("power set: ");
  Serial.print(powerSet);
  Serial.print("\t");
  Serial.print("power pwm: ");
  Serial.print(powerPWM);
  Serial.print("\t");
  Serial.print("millis: ");
  Serial.print(millis());
  Serial.println("");
}

void setup(){
  Serial.begin(115200);
  setPower(EEPROM.read(0));
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
    setLeds();
    checkButtons();
    heat();
  }
  
  delay(2);
  
  //serialDebug();
}
