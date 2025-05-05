/*
   s3servo.cpp - Library for control servo motor on esp32s3.
   Created by HA.S, October 10, 2022.
   Released into the public domain.
   mit라이선스라고하고싶은데 arduino-esp32 라이선스 따라가야겠지? 아직 라이선스 잘모르니 그냥 arduino-esp32 이거 사용"했으니 이거 따름
*/
#include "tools/s3servo.hpp"

ToneESP32::ToneESP32(int pin, int channel) {	
  this->pin = pin;
  this->channel = channel;  
  ledcAttachPin( pin, channel );
}



void ToneESP32::tone(int note) {     
	ledcSetup( channel, note, PWM_Res );
	ledcWrite( channel, 127 );                            
}


void ToneESP32::noTone() {
    ledcWrite(channel, 0);
}

s3servo::s3servo() {
}

s3servo::~s3servo() {
    detach();
}

void s3servo::detach() {
    ledcDetachPin(_pin);
}


void s3servo::reattach(){
    ledcAttachPin(_pin, _channel);
}

void s3servo::_setAngleRange(int min, int max){
    _minAngle=min;
    _maxAngle=max;
}

void s3servo::_setPulseRange(int min, int max){
    _minPulseWidth=min;
    _maxPulseWidth=max;
}

int8_t s3servo::attach(int pin, int channel , int min_angle, int max_angle, int min_pulse, int max_pulse)
{
    if(CHANNEL_MAX_NUM < channel || channel < 0){
        return -1;
    }
    _pin = pin; 
    _channel = channel;
    _setAngleRange(min_angle,max_angle);
    _setPulseRange(min_pulse,max_pulse);
    ledcSetup(channel, 50, MAX_BIT_NUM);
    ledcAttachPin(_pin, _channel);
    return 0;
}

float s3servo::mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void s3servo::write(float angle) {
    int duty = (int)mapf(angle, _minAngle, _maxAngle, _minPulseWidth, _maxPulseWidth);
    if (duty < _minPulseWidth){
        duty = _minPulseWidth;
    }
    if (duty > _maxPulseWidth){
        duty = _maxPulseWidth;
    }
    ledcWrite(_channel, duty);
}

void s3servo::writeDuty(int duty) {
    ledcWrite(_channel, duty);
}