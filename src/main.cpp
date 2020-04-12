#include <Arduino.h>
#include <avr/sleep.h>  
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GyverButton.h"  
GButton butt1(2); //Fire
GButton butt2(3); //Plus
GButton butt3(4); // Minus

int counter=0;
int wakePin = 2;
bool sleep = false;
bool fire = false;
bool power = false;
bool timeout = false;
int value = 100;
int led = 5;
int delay_t = 6000;
int i = 0;
int steps = 1;
double volt = 0;
double w = 0;
double v_dr = 0;
double curr = 0;
double curr_old = 0;
//int procents_batt = 0;
int procents = 0;
int clicks = 0;
unsigned long times;
unsigned long times_sleep;
bool pressed = false;
bool pressed_m = false;
bool pressed_p = false;

Adafruit_SSD1306 display(128, 32, &Wire, -1);

void wakeUpNow() {  
sleep_disable();  
times_sleep  = millis();
detachInterrupt(0);   
  }
 /*  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("@Lagodish"));
  display.display();
  analogWrite(9,0);
  delay(1500);*/
//delay(10);
 

void sleepNow() {  
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here  
    sleep_enable();          // enables the sleep bit in the mcucr register  
    attachInterrupt(0,wakeUpNow, LOW); // use interrupt 0 (pin 2) and run function  
    sleep_mode();            // here the device is actually put to sleep!!  
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP  
   
   // sleep_disable();         // first thing after waking from sleep: disable sleep...  
   // detachInterrupt(0);      // disables interrupt 0 on pin 2 so the wakeUpNow code will not be executed during normal running time.  
}  

void setup() {
 // Serial.begin(9600);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
   // Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
//TCCR1B = TCCR1B & B11111000 | B00000101; 
  //TCCR1B = TCCR1B & B11111000 | B00000010;
  pinMode(9,OUTPUT);
//  pinMode(led,OUTPUT);
  analogReference(INTERNAL);
  pinMode(A6,INPUT);
  pinMode(A7,INPUT);
 //batt.begin(3300, 1.49);
 //s pinMode(wakePin, INPUT_PULLUP);  
 // attachInterrupt(0, wakeUpNow, LOW); 
   
  //butt1.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
 // butt2.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  //butt3.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  butt1.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  butt1.setTimeout(500);        // настройка таймаута на удержание (по умолчанию 500 мс)
  butt1.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)

  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (PIN --- КНОПКА --- GND)
  // LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND
  butt1.setType(HIGH_PULL);
  butt2.setType(HIGH_PULL);
  butt3.setType(HIGH_PULL);

  // NORM_OPEN - нормально-разомкнутая кнопка
  // NORM_CLOSE - нормально-замкнутая кнопка
  butt1.setDirection(NORM_OPEN);
  butt2.setDirection(NORM_OPEN);
  butt3.setDirection(NORM_OPEN);
}

void loop() {
  butt1.tick();  // обязательная функция отработки. Должна постоянно опрашиваться
  butt2.tick();
  butt3.tick();
  
  if (butt1.hasClicks())                                // проверка на наличие нажатий
    {
     clicks = butt1.getClicks();
     if(clicks  == 5) {power = true;}
     if(clicks  == 3) {power = false;}
     //Serial.println(clicks);
      }

      pressed = butt1.isPress();
if(butt1.isRelease()){ fire = false; timeout = false; times_sleep  = millis();}    // отпускание кнопки (+ дебаунс)
if (pressed&&power){ fire = true; times = millis();}     // нажатие на кнопку (+ дебаунс)

if(millis() - times_sleep > 1500000){ //!!!
   timeout = false;
   power = false;
   analogWrite(9,0);
  // analogWrite(led,0);
   display.clearDisplay();
   display.display();
   sleepNow();
   }



if(power){

   // delayMicroseconds(delay_t);
  // analogWrite(led,i);

 pressed_m = butt3.isStep();
  pressed_p = butt2.isStep();
  
  if (pressed_p) {                                 // если кнопка была удержана (это для инкремента)
    value+=10;                                            // увеличивать/уменьшать переменную value с шагом и интервалом
  }
  
  if (pressed_m) {                                 // если кнопка была удержана (это для инкремента)
    value-=10;                                            // увеличивать/уменьшать переменную value с шагом и интервалом
  }

   volt = (analogRead(A6) * 1.1) / 1024.0*5.98; 
   v_dr = (analogRead(A7) * 1.1) / 1024.0;

   //volt = analogRead(A6);//temp / 0.84;

  if(value < 0){ value = 0; }

  if(value > 100){ value = 100;}


  procents = map(value, 0, 100, 100, 255);
  
     curr =  v_dr / 0.007*value/100;// * (volt-v_dr); 
   if(curr<100){
    curr_old = curr;
    }
       w = curr_old*(volt-v_dr);
  if(fire && !timeout){

    if(millis() - times > 9000){
      timeout = true;
      }
      
    if(millis() - times > 500){     
      if(volt>2.8){
  
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("!!NAPAS!!"));
  display.setTextSize(1); 
  
  display.setCursor(5, 22);
  display.println(F("V ")); //volt
  display.setCursor(12, 22);
  display.println(String(volt));
  
  display.setCursor(43, 22);
  display.println(F("C ")); //curr
  display.setCursor(50, 22);
  display.println(String(curr_old));

  display.setCursor(85, 22);
  display.println(F("W ")); //curr
  display.setCursor(92, 22);
  display.println(String(w));
  
  display.display();      // Show initial text*/
 // delay_t = 1000;
  if(millis() - times < 1500){
  analogWrite(9,255);
  }
  else{
    analogWrite(9,procents);
    }
      }
      else{
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("LOW BATT"));
   display.setTextSize(1); 
  display.setCursor(5, 22);
  display.println(F("Voltage "));
  display.setCursor(50, 22);
  display.println(String(volt));
  display.display();      // Show initial text*/
 // delay_t = 3000;
  analogWrite(9,0);
        }
    }
    }
    else{
  analogWrite(9,0);
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.println(F("Power "));
  display.setCursor(70, 0);
  display.println(String(value));
  if(value<10){  display.setCursor(90, 0);}
  if(value>9&&value<100){  display.setCursor(100, 0);}
  if(value>99){  display.setCursor(110, 0);}
  display.println(F("%"));
  display.setTextSize(1); 
  display.setCursor(5, 22);
  display.println(F("Voltage "));
  display.setCursor(50, 22);
  display.println(String(volt));
  display.display();      // Show initial text*/
  
  
}}
else{
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 15);
  display.println(F("Power off"));
  display.display();
  analogWrite(9,0);
  //analogWrite(led,0);
 // delay(100);
  }

}