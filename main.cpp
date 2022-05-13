Copyright 2018 John-TC https://github.com/john-TC

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>. 

#include <Arduino.h>
#include <Bounce2.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(2,3,4,5,6,7);

const byte sensor = 8;
const byte pot = A0;
const byte screen = 11;
const byte leftButton = 12;
const byte rightButton = A1;
const byte odoSetButton = A2;
const byte leftLed = 10;
const byte rightLed = 9;
const int interval = 424;
const byte magnets = 1;
const float circ = 2200;
const byte odoTotalAddress = 0;
const byte odoTripAddress = 4;

int potValue;
byte screenValue;
byte currentScreen;

unsigned long prevMillis;
unsigned long duration;
unsigned long leftPreviousMillis;
unsigned long rightPreviousMillis;
unsigned long odoPrevMillis;
unsigned long refresh;
unsigned long reset;

float odo;
float odo1;
float odo2;
float kmph;
float tripOdo;
float totalOdo;
float prevTotalOdo;
float prevTripOdo;

bool sensorRead = LOW;
bool prevState = LOW;
bool odoPrevState = LOW;
bool sensorState = LOW;
bool leftButtonState = LOW;
bool rightButtonState = LOW;
bool leftLedState = LOW;
bool rightLedState = LOW;
// bool rightLatch;
// bool leftLatch;

Bounce sensorBounce = Bounce();
Bounce leftBounce = Bounce();
Bounce rightBounce = Bounce();
Bounce odoSetBounce = Bounce();

void splash() {
  refresh = 3000;
  while (millis() < 3000) {
    if (millis() - refresh >= 3000) {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Live to Ride");
      lcd.setCursor(2,1);
      lcd.print("Ride to Live");
      refresh = millis();
    }
    potValue = analogRead(pot);
    screenValue = map(potValue, 0, 1023, 0, 255);
    analogWrite(screen, screenValue);
  }
}

void setup()
{
  // Serial.begin(9600);
  pinMode(pot, INPUT);
  pinMode(screen, OUTPUT);
  lcd.begin(16,2);
  // splash();
  pinMode(sensor,INPUT);
  sensorBounce.attach(sensor);
  sensorBounce.interval(10);
  pinMode(leftButton, INPUT);
  leftBounce.attach(leftButton);
  leftBounce.interval(10);
  pinMode(rightButton, INPUT);
  rightBounce.attach(rightButton);
  rightBounce.interval(10);
  pinMode(odoSetButton, INPUT);
  odoSetBounce.attach(odoSetButton);
  odoSetBounce.interval(10);
  pinMode(leftLed, OUTPUT);
  pinMode(rightLed, OUTPUT);
  prevMillis = 0;
  prevState = LOW;
}

void leftButtonDetect() {
  if (leftButtonState == LOW && leftBounce.read() == HIGH) {
    leftButtonState = HIGH;
    //rightLatch = LOW;
    //leftLatch=!leftLatch;
  }
  if (leftButtonState == HIGH && leftBounce.read() == LOW) {
    leftButtonState = LOW;
  }
}

void rightButtonDetect() {
  if (rightButtonState == LOW && rightBounce.read() == HIGH) {
    rightButtonState = HIGH;
    //leftLatch = LOW;
    //rightLatch=!rightLatch;
  }
  if (rightButtonState == HIGH && rightBounce.read() == LOW) {
    rightButtonState = LOW;
  }
}

void leftBlink() {
  if (leftButtonState == HIGH) {
    //if (leftLatch == HIGH) {
    if (millis() - leftPreviousMillis >= interval) {
      leftPreviousMillis = millis();
      leftLedState =! leftLedState;
      digitalWrite(leftLed, leftLedState);
    }
  }
  if (leftButtonState == LOW) {
    //if (leftLatch == LOW) {
    digitalWrite(leftLed, LOW);
    leftLedState = LOW;
  }
}

void rightBlink() {
  if (rightButtonState == HIGH) {
    //if (rightLatch == HIGH) {
    if (millis() - rightPreviousMillis >= interval) {
      rightPreviousMillis = millis();
      rightLedState =! rightLedState;
      digitalWrite(rightLed, rightLedState);
    }
  }
  if (rightButtonState == LOW) {
    //if (rightLatch == LOW) {
    digitalWrite(rightLed, LOW);
    rightLedState = LOW;
  }
}

void calcSpeed() {
  if (prevState != sensorBounce.read()) {
    if (prevState == LOW && sensorBounce.read() == HIGH) {
      prevState = HIGH;
      duration = millis() - prevMillis;
      kmph = (circ / duration * 3.6 / magnets);
      odo = odo + (circ / magnets);
      odo1 = odo / 1000;
      odo2 = odo1 / 1000;
      prevMillis = millis();
      reset = millis();
    }
    if (prevState == HIGH && sensorBounce.read() == LOW) {
      prevState = LOW;
    }
  }
  if (kmph == kmph && millis() - reset > 2667 / magnets) {
    kmph = 0;
    reset = millis();
  }
}

void setOdo() {
  if (odoPrevState != odoSetBounce.read()) {
    if (odoPrevState == LOW && odoSetBounce.read() == HIGH) {
      odoPrevState = HIGH;
      odoPrevMillis = millis();
    }
  }
  if (currentScreen == 2 && odoPrevState == HIGH && odoSetBounce.read() == HIGH && millis() - odoPrevMillis >= 2000) {
    if (prevTotalOdo != odo2) {
      totalOdo = EEPROM.get(odoTotalAddress, totalOdo) + odo2 - prevTotalOdo;
      prevTotalOdo = odo2;
      EEPROM.put(odoTotalAddress, totalOdo);
    }
  }
  if (currentScreen == 1 && odoPrevState == HIGH && odoSetBounce.read() == HIGH && millis() - odoPrevMillis >= 5000) {
    EEPROM.put(odoTripAddress, 0.00);
  }
  if (currentScreen == 1 && odoPrevState == HIGH && odoSetBounce.read() == HIGH && millis() - odoPrevMillis >= 2000) {
    if (prevTripOdo != odo2) {
      tripOdo = EEPROM.get(odoTripAddress, tripOdo) + odo2 - prevTripOdo;
      prevTripOdo = odo2;
      EEPROM.put(odoTripAddress, tripOdo);
    }
  }
  if (odoPrevState == HIGH && odoSetBounce.read() == LOW) {
    odoPrevState = LOW;
    if (currentScreen == 0 && millis() - odoPrevMillis < 2000) {
      currentScreen = 1;
    }
    else if (currentScreen == 1 && millis() - odoPrevMillis < 2000) {
      currentScreen = 2;
    }
    else if (currentScreen == 2 && millis() - odoPrevMillis < 2000) {
      currentScreen = 3;
    }
    else if (currentScreen == 3 && millis() - odoPrevMillis < 2000) {
      currentScreen = 0;
    }
  }
}

void screenCurrentOdo() {
  if (currentScreen == 0) {
    if (odo2 < 10) {
      lcd.print("Current   ");
    }
    else if (odo2 >= 10) {
      lcd.print("Current  ");
    }
    lcd.print(odo2);
    lcd.print("km");
  }
}

void screenTripOdo() {
  if (currentScreen == 1) {
    if (tripOdo + odo2 < 10) {
      if (EEPROM.get(odoTripAddress, tripOdo) + odo2 - prevTripOdo != EEPROM.get(odoTripAddress, tripOdo)){
        lcd.print("Trip*     ");
      }
      else {
        lcd.print("Trip      ");
      }
    }
    else if (tripOdo + odo2 >= 10) {
      if (EEPROM.get(odoTripAddress, tripOdo) + odo2 - prevTripOdo != EEPROM.get(odoTripAddress, tripOdo)){
        lcd.print("Trip*    ");
      }
      else {
        lcd.print("Trip     ");
      }
    }
    else if (tripOdo + odo2 >= 100) {
      if (EEPROM.get(odoTripAddress, tripOdo) + odo2 - prevTripOdo != EEPROM.get(odoTripAddress, tripOdo)){
        lcd.print("Trip*   ");
      }
      else {
        lcd.print("Trip    ");
      }
    }
    lcd.print(EEPROM.get(odoTripAddress, tripOdo) + odo2 - prevTripOdo);
    lcd.print("km");
  }
}

void screenTotalOdo() {
  if (currentScreen == 2) {
    if (totalOdo + odo2 < 10) {
      if (EEPROM.get(odoTotalAddress, totalOdo) + odo2 - prevTotalOdo != EEPROM.get(odoTotalAddress, totalOdo)){
        lcd.print("Total*    ");
      }
      else {
        lcd.print("Total     ");
      }
    }
    else if (totalOdo + odo2 >= 10) {
      if (EEPROM.get(odoTotalAddress, totalOdo) + odo2 - prevTotalOdo != EEPROM.get(odoTotalAddress, totalOdo)){
        lcd.print("Total*   ");
      }
      else {
        lcd.print("Total    ");
      }
    }
    else if (totalOdo + odo2 >= 100) {
      if (EEPROM.get(odoTotalAddress, totalOdo) + odo2 - prevTotalOdo != EEPROM.get(odoTotalAddress, totalOdo)){
        lcd.print("Total*  ");
      }
      else {
        lcd.print("Total   ");
      }
    }
    lcd.print(EEPROM.get(odoTotalAddress, totalOdo) + odo2 - prevTotalOdo);
    lcd.print("km");
  }
}

void screenElapsedTime() {
  if (currentScreen == 3) {
    float h, m, s;
    unsigned long over;
    h = int(millis() / 3600000);
    over = millis() % 3600000;
    m = int(over / 60000);
    over = over % 60000;
    s = int(over / 1000);
    lcd.print("Time    ");
    if (h<10) lcd.print('0');
    lcd.print(h, 0);
    lcd.print(":");
    if (m<10) lcd.print('0');
    lcd.print(m, 0);
    lcd.print(":");
    if (s<10) lcd.print('0');
    lcd.print(s, 0);
  }
}

void display() {
  if (millis() - refresh >= 500) {
    lcd.clear();
    lcd.setCursor(4,0);
    if (kmph<10) lcd.print('0');
    lcd.print(kmph);
    lcd.print("kmh");
    lcd.setCursor(0,1);
    screenCurrentOdo();
    screenTripOdo();
    screenTotalOdo();
    screenElapsedTime();
    refresh = millis();
  }
}

void screenBrightness() {
  potValue = analogRead(pot);
  screenValue = map(potValue, 0, 1023, 0, 255);
  analogWrite(screen, screenValue);
}

void loop()
{
  sensorBounce.update();
  leftBounce.update();
  rightBounce.update();
  odoSetBounce.update();
  setOdo();
  calcSpeed();
  display();
  leftButtonDetect();
  rightButtonDetect();
  leftBlink();
  rightBlink();
  screenBrightness();
}
