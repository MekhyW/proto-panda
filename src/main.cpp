#include "config.hpp"

#include <SPI.h>
#include "SD.h"
#include <string>
#include <FastLED.h>


#include "tools/sensors.hpp"
#include "tools/devices.hpp"
#include "tools/oledscreen.hpp"
#include "tools/storage.hpp"
#include "tools/logger.hpp"
#include "lua/luainterface.hpp"


#include "drawing/framerepository.hpp"
#include "drawing/dma_display.hpp"
#include "drawing/animation.hpp"
#include "drawing/ledstrip.hpp"
#include "drawing/icons/icons.hpp"


#include "editmode/editmode.hpp"

#include "ble_client.hpp"


LedStrip g_leds;
FrameRepository g_frameRepo;
BleManager g_remoteControls;
Animation g_animation;
LuaInterface g_lua;
TaskHandle_t g_secondCore;
TaskHandle_t g_firstCore;
EditMode g_editMode;

void second_loop(void*);

void setup() {
  /*
    Startup regulator pins and shoot it low asap.
  */
  #ifdef PIN_ENABLE_REGULATOR
  digitalWrite(PIN_ENABLE_REGULATOR, LOW);
  pinMode(PIN_ENABLE_REGULATOR, OUTPUT);
  digitalWrite(PIN_ENABLE_REGULATOR, LOW);
  #endif
  pinMode(EDIT_MODE_PIN, INPUT_PULLDOWN);
  pinMode(PIN_USB_BATTERY_IN, INPUT);

  Devices::Begin();
  Serial.begin(115200);
  Serial.printf("Starting proto panda v%s!\n", PANDA_VERSION);


  Devices::BuzzerTone(2220);
  delay(200);
  Devices::BuzzerNoTone();
  Devices::I2CScan();
  Devices::BuzzerTone(1220);
  delay(200);
  Devices::BuzzerNoTone();
 
  OledScreen::Start();
  Sensors::Start();
  
  g_editMode.CheckBeginEditMode();

  if (g_editMode.IsOnEditMode()){
    Devices::BuzzerTone(220);
    delay(200);
    Devices::BuzzerTone(220);
    delay(200);
    Devices::BuzzerNoTone();
    return;
  }

  while (!Storage::Begin()){
    OledScreen::display.clearDisplay();
    OledScreen::display.drawBitmap(0,0, icon_sd, 128, 64, 1);
    OledScreen::display.display();
    delay(500);
    OledScreen::display.clearDisplay();
    OledScreen::display.display();
  }

  Logger::Begin();
  Devices::DisplayResetInfo();
  Devices::I2CScan();
  Devices::StartAvaliableDevices();

  Devices::CalculateMemmoryUsage();  
  if (!g_frameRepo.Begin()){
    OledScreen::CriticalFail("Frame repository has failed! If restarting does not solve, its a hardware problem.");
    for (;;){
      Devices::BuzzerTone(420);
      delay(200);
      Devices::BuzzerTone(420);
      delay(200);
      Devices::BuzzerNoTone();
      delay(1000);
    }
  }
  Devices::CalculateMemmoryUsage(); 
  if (!g_lua.Start()){
    OledScreen::CriticalFail("Failed to initialize Lua!");
    for(;;){
      Devices::BuzzerTone(420);
      delay(200);
      Devices::BuzzerTone(420);
      delay(200);
      Devices::BuzzerNoTone();
      delay(1000);
    }
  }
  Devices::CalculateMemmoryUsage(); 
  if (!DMADisplay::Start()){
    OledScreen::CriticalFail("Failed to initialize DMA display!");
    Devices::BuzzerTone(300);
    delay(1500);
    Devices::BuzzerNoTone();
    for(;;){}
  }
  Logger::Info("DMA display initialized!");

  Devices::CalculateMemmoryUsage(); 
  if (!g_lua.LoadFile("/init.lua")){
    OledScreen::CriticalFail("Failed to load init.lua");
    Devices::BuzzerTone(300);
    delay(1500);
    Devices::BuzzerNoTone();
    for(;;){}
  }

  Devices::CalculateMemmoryUsage();

  OledScreen::SetConsoleMode(false);
  OledScreen::display.setCursor(0,0);
  OledScreen::display.setTextSize(2);
  OledScreen::display.printf("Starting\nLua");
  OledScreen::display.display();
  OledScreen::display.setTextSize(1);
  Logger::Info("Starting Lua");
  g_lua.CallFunction("onSetup");
  Devices::BuzzerTone(150);
  delay(100);
  Devices::BuzzerNoTone();
  Devices::CalculateMemmoryUsage();


  g_frameRepo.displayFFATInfo();
  Serial.printf("Running upon %d\n", xPortGetCoreID());
  
  xTaskCreatePinnedToCore(second_loop, "second loop", 10000, NULL, ( 2 | portPRIVILEGE_BIT ), &g_secondCore, 0);

  Devices::CalculateMemmoryUsage();  
  Devices::BuzzerTone(880);
  delay(100);
  g_lua.CallFunction("onPreflight");
  Devices::BuzzerNoTone();
  
  Devices::CalculateMemmoryUsage();
}

void second_loop(void*){
  
  for( ;; )
  { 
    uint32_t st = millis();
    Devices::BeginAutoFrame();
    g_animation.Update(g_frameRepo.takeFile());
    vTaskDelay(1);
    uint32_t st2 = millis()-st;
    g_frameRepo.freeFile();
    g_leds.Update();
    if (g_leds.IsManaged()){
      g_leds.Display();
    }
    vTaskDelay(1);
    Devices::EndAutoFrame();
    st = millis()-st;
    if (st > 80){
      Logger::Info("Animation cycle took too long %d and %d ms", st2, st);
    }
    vTaskDelay(3);
  }
}


void loop() {
  if (g_editMode.IsOnEditMode()){
    g_editMode.LoopEditMode();
    return;
  }
  

  if (Devices::AutoCheckPowerLevel() && !Devices::CheckPowerLevel()){
    Devices::WaitForPower(0);
    return;
  }

  Devices::BeginFrame();
  Devices::ReadSensors();
  g_remoteControls.update();
  g_remoteControls.updateButtons();
  g_lua.CallFunctionT("onLoop", Devices::getDeltaTime());
  Devices::EndFrame();  
}
