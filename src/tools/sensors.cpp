#include "tools/sensors.hpp"


uint16_t Sensors::voltages[MEASUREMENTS];
uint16_t Sensors::pointer = 0;


void Sensors::Start(){
    FullMeasureVoltage();   
}

uint16_t Sensors::MeasureVoltage(){
    uint16_t reading = analogRead(PIN_USB_BATTERY_IN);
    Sensors::voltages[pointer++] = reading;
    if (pointer == (MEASUREMENTS-1)){
        pointer = 0;
    }
    return reading;
}

void Sensors::FullMeasureVoltage(){
    for (int i=0;i<MEASUREMENTS;i++){
        Sensors::MeasureVoltage();
        delay(2);
    }
}

float Sensors::GetBatteryVoltage(){
    return Sensors::calculateVoltage(Sensors::MeasureVoltage());
}

float Sensors::GetAvgBatteryVoltage(){
    uint32_t val = 0;
    for (int i=0;i<MEASUREMENTS;i++){
        val += Sensors::voltages[i];
    }
    return Sensors::calculateVoltage(val/(float)MEASUREMENTS);
}

float Sensors::calculateVoltage(uint16_t val){
    return val * (V_REF / 4095.0) * ((RESISTOR_DIVIDER_R8 + RESISTOR_DIVIDER_R9) / RESISTOR_DIVIDER_R9);
}