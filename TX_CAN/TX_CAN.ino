

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "AbsoluteEncoder.h"
#include "Battery.h"
#include "DynamixelSerial.h"
#include "TractionEncoder.h"
#include "MovingAvgFilter.h"
#include "ExpSmoothingFilter.h"
#include "Debug.h"
#include "mcp2515.h"
#include "Display.h"
#include "SmartMotor.h"
#include "Motor.h"
#include "PID.h"
#include "include/definitions.h"
#include "include/mod_config.h"
#include "include/communication.h"
#include "CanWrapper.h"
#include "Dynamixel_ll.h"



CanWrapper canW(5, 10000000UL, &SPI);

float oldAngle;

bool automaticSend= false; // if true, the angle is sent automatically, otherwise it is sent only when requested by the master





float autoinput = 0.0; // this is the value that is sent automatically, it can be changed by the master
float autoinputCalcolate();
unsigned long timer = 0;
void setup()
{
   Debug.setLevel(Levels::INFO); // comment to set debug verbosity to debug
  Wire1.setSDA(I2C_SENS_SDA);
  Wire1.setSCL(I2C_SENS_SCL);
  Wire1.begin();

  SPI.setRX(4);
  SPI.setCS(5);
  SPI.setSCK(6);
  SPI.setTX(7);
  SPI.begin();



  // CAN initialization
  canW.begin();

};

void loop()
{
  if(automaticSend){
  float speeds[2] = {autoinput,autoinput};
    canW.sendMessage(MOTOR_SETPOINT, speeds, 8);



    delay (10); // send every 100ms
    if (millis() - timer > 2500){

       autoinput = autoinputCalcolate();
      Serial.print("Autoinput: ");
      Serial.println(autoinput);
      timer = millis();
    }

  }else if (Serial1.available() >= 9) {
    if (Serial1.read() == 0xAA) {      // verifica il marker di inizio
      uint8_t buffer[8];
      Serial1.readBytes(buffer, 8);    // leggi 8 byte (2 float)
      // Ricostruisci i float da 4 byte little-endian
      union { uint8_t b[4]; float f; } conv;
      memcpy(conv.b, buffer, 4);
      float vel_left  = conv.f;
      memcpy(conv.b, buffer+4, 4);
      float vel_right = conv.f;
      // Prepara pacchetto CAN con i due float
      uint8_t canData[8];
      memcpy(canData, buffer, 8);
      // Invia il messaggio CAN all'ID definito
      canW.sendMessage(MOTOR_SETPOINT, canData, 8);
    }
  }

  delay(10);
}


float autoinputCalcolate() {


  float newValue = random(0, 50) / 1.0;

  return newValue; // Return the old value if the new value is the same
}