#line 1 "C:\\Users\\Titania\\Desktop\\isaac\\prova_0705\\oggi\\PLL_JOISTICK_CAN\\TX_CAN\\lib\\SmartMotor\\src\\SmartMotor.h"
#ifndef SMART_MOTOR_H
#define SMART_MOTOR_H

#include <Arduino.h>

#include "Motor.h"
#include "TractionEncoder.h"
#include "PID.h"
#include "MovingAvgFilter.h"
#include "Debug.h"
#include <Adafruit_ADS1X15.h>
/**
 * Class used to control DC motors at a constant speed.
 * Relies on a PID controller receiving data from a rotary encoder that uses a PIO state machine to reduce CPU usage and increase reliability.
 */
class SmartMotor
{
public:
  SmartMotor(byte pwm, byte dir, byte enc_a, byte enc_b, Adafruit_ADS1115 &adc, int base_adc_channel, bool invert = false, PIO pio = pio0);
  void begin();
  void update();

  void setSpeed(float value);
  float getSpeed();
  float getCurrent();
  float getTemperature();
  void stop();

  void setPIDvariables();
  void calibrate(float target = 45.f);
  void setCurrentControl(bool status) ;
  void setTemperatureControl(bool status);
  bool getCurrentControl() ;
  bool getTemperatureControl() ;

  bool isCalibrated() ;

private:
  int speedToPower(float speed);

  Motor motor;
  TractionEncoder encoder;
  PID pid;


  float speed;
  unsigned long enc_last;
  unsigned long pid_last;


      Adafruit_ADS1115& _adc;
    int base_adc_channel;
    bool invert;

    float current;
    float temperature;
    unsigned long enc_last_speed;
    unsigned long current_last;
    unsigned long temperature_last;

    int overCurrentCount = 0;
    int overTemperatureCount = 0;
    bool motorCurrWarning = 0;
    bool motorTempWarning = 0;
    int safe_mode = 0; // 0: no safe mode, 1: temperature is too high, 2: current is too high
    float division_factor = 1.f; // factor to divide the speed when the current is too high
    bool _CurrentControl = false; // enable current control
    bool _TemperatureControl = false; // enable temperature control
    bool isReached = false;
};

#endif
