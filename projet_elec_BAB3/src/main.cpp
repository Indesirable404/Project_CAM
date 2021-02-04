#include <Arduino.h>

#define Flashlight 4

void setup() {
  // put your setup code here, to run once:
  pinMode(Flashlight, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  digitalWrite(Flashlight, HIGH);
  delay(1000);
  digitalWrite(Flashlight, LOW);
}