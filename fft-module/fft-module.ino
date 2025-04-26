#include <Arduino.h>
#include "arduinoFFT.h"

const uint16_t samples = 256; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 4000; //Hz, must be less than 10000 due to ADC
unsigned int sampling_period_us;
unsigned int microseconds;
volatile int activeBuffer = 0;
volatile bool newDataAvailable = false;

static char output[64*4];
double vReal[samples]; 
double vReal2[samples];
double vImag[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFrequency);

void setup() {
  Serial2.begin(921600, SERIAL_8N1, 12, 13);
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  xTaskCreatePinnedToCore(readTask, "DMA_Read", 4096, NULL, 1, NULL, 1);
}

void readTask(void *parameter) {
  size_t bytesRead;
  activeBuffer = 0;
  while (1) {
    while(newDataAvailable){}
    auto used = vReal;
    if (activeBuffer == 1){
      used = vReal2;
    }
    for(int i=0; i<samples; i++) {
      microseconds = micros();  
      used[i] = analogRead(4);
      while(micros() < (microseconds + sampling_period_us)){}
    }
    activeBuffer++;
    if (activeBuffer > 1){
      activeBuffer = 0;
    }
    newDataAvailable = true;
    vTaskDelay(1);
  }
}
uint32_t lastDt = 0;
void loop() {
  if (newDataAvailable) {
    auto kek = vReal;
    if (activeBuffer == 0){
      kek = vReal2;
    }
    newDataAvailable = false;
    memset(vImag, 0, samples * sizeof(double));
    FFT.setArrays(kek , vImag);
    FFT.dcRemoval();
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();

    char* ptr = output;
    int windows = samples/128;
    for (int i = 0; i < samples/2; i+=windows) {
      int barHeight = map(kek[i], 0, 1024, 0, 255);
        if (barHeight > 255){
          barHeight = 255;
        }
        ptr += sprintf(ptr, "%02x", barHeight);
    }
    *ptr = '\n';
    ptr++;
    *ptr = '\0';   
    if (activeBuffer == 0){
      Serial2.setTimeout(1);
      Serial2.write(output);
    }
    delay(55);
  }
};

