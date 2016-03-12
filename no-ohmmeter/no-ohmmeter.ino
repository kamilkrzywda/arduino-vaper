#include <EEPROM.h>

/*
 * settings
 */
const int sleepTime = 30000;     //sleep timer
const int preheatTime = 200;      //preheat boost time

const int buttonsSensitivity = 900;  //touch buttons sensitivity
const int batteryCal = 37;             //battery callibration value

/* 
 *  pin config
 */
const int batterySensor = A1;   //batery voltage sensors pin
const int button1 = A2;         //button 1
const int button2 = A3;         //button 2
const int heaterButton = 9;     //heater button pin
const int heaterOutput = 10;    //heater mosfet pin
const int led1 = 3;             //led pin 1
const int led2 = 5;             //led pin 2
const int led3 = 6;             //led pin 3

//some app vars
int led1Light = 0;
int led2Light = 0;
int led3Light = 0;

int button1Status = 0;
int button2Status = 0;
int heaterButtonStatus = 0;

int batterySensorValue = 0;
float batteryVoltage = 8.00;
float voltageCorrection = 1.00;

int powerSet = 0;
int powerPWM = 0;

int heating = 0;
int sleep = 0;

/*
 * change  power setting
 */
void setPower(int power){
  powerSet = power;
  if(powerSet > 192){
    powerSet = 192;
    powerPWM = 255;
  }else if(powerSet < 5){
    powerSet = 5;
  }else{
    powerPWM = powerSet;
  }
  EEPROM.write(0, powerSet);
  setLeds();
}

void updateLeds(){
  led1Light = constrain(led1Light,0,255);
  led2Light = constrain(led2Light,0,255);
  led3Light = constrain(led3Light,0,255);
  analogWrite(led1,led1Light);
  analogWrite(led2,led2Light);
  analogWrite(led3,led3Light);
}

void setLeds(){
  if(powerSet <= 63){
    led1Light = powerSet*powerSet/16;
    led2Light = 0;
    led3Light = 0;
  }else if(powerSet <= 127){
    led1Light = 255;
    led2Light = (powerSet-64)*(powerSet-64)/16;
    led3Light = 0;
  }else if(powerSet <= 191){
    led1Light = 255;
    led2Light = 255;
    led3Light = (powerSet-128)*(powerSet-128)/16;
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
    if(sleep > sleepTime){
      showBatteryStatus(100);
    }
    sleep = 0;
  }
  
  if(heating == 0){
    batterySensorValue = analogRead(batterySensor);
    batteryVoltage = (float)batterySensorValue/batteryCal;
  }
}

/*
 * buttons actions
 */
void checkButtons(){
  if((button1Status < 1000) &&  (button2Status < 1000)){
    showBatteryStatus(1000);
  }
  
  if(button1Status < 1000){
    sleep = 0; 
    setPower(powerSet+1);
    delay(10);
  }
  
  if(button2Status < 1000){
    sleep = 0;
    setPower(powerSet-1);
    delay(10);
  }
  
  if(heaterButtonStatus){
    heating += 1;
  }else{
    if(heating > 0){
      heating = 0;
      setLeds();
    }
  }
}

void showBatteryStatus(int showTime){
  led1Light = (batteryVoltage-7)*255;
  led2Light = (batteryVoltage-8)*255;
  led3Light = (batteryVoltage-9)*255;
  updateLeds();
  delay(showTime);
  led1Light = 0;
  led2Light = 0;
  led3Light = 0;
  updateLeds();
  delay(10);
  setLeds();
}

/*
 * heating
 */
void heat(){
  if(heating > 0){
    if(batteryVoltage > 7){
      //boost power at start to preheat coil
      voltageCorrection = (7.0/batteryVoltage)*(7.0/batteryVoltage);
      if(heating < preheatTime){
        analogWrite(heaterOutput,constrain(255-powerPWM*voltageCorrection*2,0,255));
      }else{
        analogWrite(heaterOutput,255-powerPWM*voltageCorrection);
      }
    }else{
      //battery low alert
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
      setLeds(); 
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
  //disable heating at start
  digitalWrite(heaterOutput,HIGH);
}

void loop() {
  sleep++;
  
  readStatus();

  if(sleep == sleepTime){
    while(led1Light != led2Light != led3Light != 0){
      led1Light--;
      led2Light--;
      led3Light--;
      updateLeds();
      delay(3);
    }
    led1Light = 0;
    led2Light = 0;
    led3Light = 0;
    updateLeds();
  }else if(sleep < sleepTime){
    checkButtons();
    heat();
  }
  if(powerPWM == 255){
    setLeds(); 
  }
  delay(2);
  
  //serialDebug();
}
