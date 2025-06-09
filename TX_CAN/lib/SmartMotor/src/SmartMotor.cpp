#line 1 "C:\\Users\\PC1\\Desktop\\ISAAC\\test_pid\\PicoLowLevel\\lib\\SmartMotor\\src\\SmartMotor.cpp"
#include "SmartMotor.h"
extern bool setPIDDebug;
extern bool setCurrentDebug;
extern bool setTemperatureDebug;
extern int motor_num;
/**
 * Create SmartMotor object, creating all necessary objects.
 * @param pwm PWM pin.
 * @param dir Direction pin.
 * @param enc_a Pin A of the encoder.
 * @param enc_b Pin B of the encoder.
 * @param invert Invert motor direction, usuful when motors are mounted opposite to one another.
 * @param pio PIO to use for the encoder. Each PIO can handle up to 4 encoders.
 */
SmartMotor::SmartMotor(byte pwm, byte dir, byte enc_a, byte enc_b, Adafruit_ADS1115 &adc, int base_adc_channel, bool invert, PIO pio)
    : motor(pwm, dir, invert),
      encoder(enc_a, enc_b, new MovingAvgFilter<int>(ENC_TR_SAMPLES), invert, pio),
      _adc(adc),
      base_adc_channel(base_adc_channel),
      pid(0.f, 0.f, 0.f, MAX_SPEED, 1.f),
      invert(invert)
{
}

/**
 * Initialize SmartMotor and necessary components.
 */
void SmartMotor::begin()
{
    motor.begin();
    encoder.begin();
    analogReadResolution(12);
}

/**
 * Update routine, updating the PID and the motor speed.
 * This function will be executed at a fixed rate, defined by DT_PID, and should therefore be called as often as possible.
 */
void SmartMotor::update()
{
    unsigned long now = millis();
    if (now - pid_last > DT_PID)
    {
        if (setPIDDebug)
        {
            if (motor_num == 1)
            {
                Serial.print(" \tmillis\t");
                Serial.print(millis());

                // update motors
                Serial.print("\tMOTOR_LEFT\t");
            }
            else if (motor_num == 2)
            {
                Serial.print(" \tmillis\t");
                Serial.print(millis());

                // update motors
                Serial.print("\tMOTOR_RIGHT\t");
            }
            Serial.print("\tgetReferenceValue\t");
            Serial.print(pid.getReferenceValue());
            Serial.print("\tgetSpeed\t");
            Serial.print(getSpeed());
        }
        pid.updateFeedback(getSpeed());
        pid.calculate();
        motor.write(speedToPower(pid.getOutput()));
        if (setPIDDebug)
        {
            Serial.print("\tgetOutput\t");
            Serial.print(pid.getOutput());
            Serial.print("\tspeedtoPower\t");
            // Serial.print(pr);
            // pr++;
            Serial.println(speedToPower(pid.getOutput()));
        }
        pid_last = now;
    }
}

void SmartMotor::setCurrentControl(bool status)
{
    _CurrentControl = status;
}

void SmartMotor::setTemperatureControl(bool status)
{
    _TemperatureControl = status;
}

bool SmartMotor::getCurrentControl()
{
    return _CurrentControl;
}

bool SmartMotor::getTemperatureControl()
{
    return _TemperatureControl;
}

/**
 * Get the current speed of the motor.
 * The value is only updated at a fixed rate, defined by DT_ENC, to avoid losing precision.
 * @return float Current speed of the motor between -MAX_SPEED and MAX_SPEED.
 */
float SmartMotor::getSpeed()
{
    unsigned long now = millis();
    if (now - enc_last > DT_ENC)
    {
        speed = (float)(encoder.getSpeed()) / 100.f;
        enc_last = now;
    }
    return speed;
}

/**
 * Set the desired speed of the motor.
 * @param value Desired motor speed between -MAX_SPEED and MAX_SPEED.
 */
void SmartMotor::setSpeed(float value)
{
    float safe_value;
    if (_CurrentControl || _TemperatureControl)
    {

        switch (safe_mode)
        {
            {
            case 0: // No safe mode, set the speed directly
                safe_value = value;
                break;
            case 1: // Temperature is too high, set the speed to 0
                safe_value = 0.f;
                break;
            case 2: // Current is too high, set the speed to a reduced value
                safe_value = value / division_factor;
                break;
            }
            pid.updateReferenceValue(safe_value);
        }
    }
    else
    {
        // If current or temperature control is enabled, set the speed to 0
        pid.updateReferenceValue(value);
    }
}
/**
 * Stop the motor.
 * This function will stop the motor and reset the PID.
 */
void SmartMotor::stop()
{
    motor.write(0);
    pid.updateReferenceValue(0.f);
    pid.resetState();
}

/**
 * Calibrate the PID controller con numeri trovati sperimentalmente
 */
void SmartMotor::setPIDvariables()
{
#if defined(MK2_MOD1)
    pid.setKp(0.35f);
    pid.setKi(1.0f);
    pid.setKd(0.15f);
#endif
#if defined(MK2_MOD2) // TODO: CAMBIARE CON I VALORI REALI
    pid.setKp(0.35f);
    pid.setKi(1.0f);
    pid.setKd(0.15f);
#endif
}

/**
 * Calibrate the PID controller.
 * This function will set the PID parameters to values that should work for the motor.
 * The method used is based on Åström–Hägglund tuning method, while using Ziegler-Nichols formulas to compute the gains.
 * Only the Kp and Ki gains are computed while the Kd gain is set to 0 since it doesn't have a positive effect on controlling the motor.
 * @param target Target speed to use for calibration.
 */
void SmartMotor::calibrate(float target) {
    float th = target + 5.f;
    float tl = target - 5.f;

    motor.write(PWM_MAX_VALUE);
    while(getSpeed() < th){
       // Serial.println("th loop");
         delay(DT_ENC);
    }
    if (getSpeed() >= th) {
        isReached = true;
    }
    int t_high = millis();
    float val_high = getSpeed();
    motor.write(0);
    while(getSpeed() > tl) {
        //Serial.println("tl loop");
         delay(DT_ENC);
    }
    int t_low = millis();
    float val_low = getSpeed();

    float tu = (float)(t_low - t_high)/1000.f;
    float amplitude = (val_high - val_low)/2.f;
    float ku = 4.f * MAX_SPEED / (PI * amplitude);

    float ti = 0.83f * tu;

    float kp = 0.45f * ku * 0.1f;
    float ki = kp / ti;

    Debug.println("Motor calibration result: Kp " + String(kp) + ", Ki " + String(ki), Levels::INFO);

    pid.setKp(kp);
    pid.setKi(ki);
}

/**
 * Converts speed to power.
 * This function linearly scales the speed to the PWM duty cycle, without taking into account the motor's.
 * @param speed Theretical speed of the motor.
 * @return int PWM value to set the motor to.
 */
int SmartMotor::speedToPower(float speed)
{
    return (speed / MAX_SPEED) * PWM_MAX_VALUE;
}

/**
 * Get the current flowing in the motor.
 *
 * @return float Current
 */
float SmartMotor::getCurrent()
{
    unsigned long now = millis();
    if (now - current_last > DT_MOTOR_CURR)
    {
        int rawValue = _adc.readADC_SingleEnded(base_adc_channel + 1); // Read the ADC value once
        float voltage = (rawValue * 3.3) / 4096.0;
        current = (float)(voltage - 2.5) / 0.185; // Calculate current in amperes
        current_last = now;
        if(setCurrentDebug){
            if (motor_num == 1)
            {
                Serial.print(" \tmillis\t");
                Serial.print(millis());
                Serial.print("\tMOTOR_LEFT\t");
            }
            else if (motor_num == 2)
            {
                Serial.print(" \tmillis\t");
                Serial.print(millis());
                Serial.print("\tMOTOR_RIGHT\t");
            }
            Serial.print("\rawValue\t");
            Serial.print(rawValue);
            Serial.print("\tvoltage\t");
            Serial.print(voltage);
            Serial.print("\tcurrent\t");
            Serial.print(current);
        }
    }
    // Controlling overcurrent
    if (current > MAX_CURR)
    {
        overCurrentCount++;
        if (overCurrentCount >= MAX_CURR_READINGS)
        {
            motorCurrWarning = 1;
            safe_mode = 2;
            division_factor = 2.f; // Reduce the speed by half
        }
    }
    else if (current > MAX_CURR - 1)
    {
        overCurrentCount++;
        if (overCurrentCount >= MAX_CURR_READINGS * 2)
        {
            motorCurrWarning = 1;
            safe_mode = 2;
            division_factor = 1.5f; // Reduce the speed by 1/3
        }
    }
    else if (current > MAX_CURR - 2)
    {
        overCurrentCount++;
        if (overCurrentCount >= MAX_CURR_READINGS * 3)
        {
            motorCurrWarning = 1;
            safe_mode = 2;
            division_factor = 1.25f; // Reduce the speed by 1/4
        }
    }
    else
    {
        overCurrentCount = 0;
    }
    if (motorCurrWarning == 1)
    {
        if (current < MAX_CURR - 2.3)
        {
            motorCurrWarning = 0;
            safe_mode = 0;
        }
    }
            if(setCurrentDebug){
           Serial.print("\tmotorCurrWarning\t");
              Serial.print(motorCurrWarning);
              Serial.print("\toverCurrentCount\t");
                Serial.print(overCurrentCount);
                Serial.print("\tsafe_mode\t");
                Serial.println(safe_mode);
        }
    return current;
}

/**
 * Get the temperature of the motor.
 *
 * @return float temperature in Celsius
 */

float SmartMotor::getTemperature()
{
    unsigned long now = millis();

    int rawValue = _adc.readADC_SingleEnded(base_adc_channel);
    float vout = (float)(rawValue / 5502.37f); //  N.B: this value has been found by meausing the voltage on the thermistor
    // Serial.println("Raw ADC Value: " + String(rawValue) + ", Vout: " + String(vout));
    float Rntc = vout * 100000.0f / (3.3f - vout);                           //  Rntc = vout * Rf / (vin - vout);
    temperature = (4450.0f / (log(Rntc / 100000.0f) + (4450.0f / 298.15f))); // B / (log(Rntc / R0) + (B / T0));
    temperature = temperature - 273.15f;
    temperature_last = now;
            if(setTemperatureDebug){
            if (motor_num == 1)
            {
                Serial.print(" \tmillis\t");
                Serial.print(millis());
                Serial.print("\tMOTOR_LEFT\t");
            }
            else if (motor_num == 2)
            {
                Serial.print(" \tmillis\t");
                Serial.print(millis());
                Serial.print("\tMOTOR_RIGHT\t");
            }
           Serial.print("\trawValue\t");
            Serial.print(rawValue);
            Serial.print("\tvout\t");
            Serial.print(vout);
            Serial.print("\tRntc\t");
            Serial.print(Rntc);
            Serial.print("\ttemperature\t");
            Serial.println(temperature);
        }

    // Controlling overtemperature
    if (temperature > MAX_TEMP)
    {
        overTemperatureCount++;
        if (overTemperatureCount >= MAX_TEMP_READINGS)
        {
            motorTempWarning = 1;
            safe_mode = 1;
        }
    }
    else
    {
        overTemperatureCount = 0;
    }
    if (motorTempWarning == 1)
    {
        if (temperature < MAX_TEMP - 15)
        {
            motorTempWarning = 0;
            safe_mode = 0;
        }
    }
    if (setTemperatureDebug)
    {
        Serial.print("\tmotorTempWarning\t");
        Serial.print(motorTempWarning);
        Serial.print("\toverTemperatureCount\t");
        Serial.print(overTemperatureCount);
        Serial.print("\tsafe_mode\t");
        Serial.println(safe_mode);
    }
    return temperature;
}

bool SmartMotor::isCalibrated() {
    return isReached;
}