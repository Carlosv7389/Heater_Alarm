/*Blink Code to test the heater circuit*/
#include <Arduino.h>
#define Builtin_LED 2 // Indicator LED for the MCU
#define data_line 17 // Pin for sending the activation signal
void setup() {
  delay(100);
  pinMode(Builtin_LED, OUTPUT);      // set the LED pin mode
  pinMode(17, OUTPUT);
}
void loop(){
  delay(3000);
  digitalWrite(Builtin_LED, HIGH);
  digitalWrite(17, HIGH);
  delay(3000);
  digitalWrite(Builtin_LED, LOW);
  digitalWrite(17, LOW);
}