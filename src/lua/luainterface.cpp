#include "lua/luainterface.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "tools/sensors.hpp"
#include "tools/oledscreen.hpp"
#include "tools/storage.hpp"
#include "drawing/dma_display.hpp"
#include "drawing/framerepository.hpp"
#include "drawing/animation.hpp"
#include "drawing/ledstrip.hpp"
#include "ble_client.hpp"

extern FrameRepository g_frameRepo;
extern Animation g_animation;
extern LedStrip g_leds;

static PersistentDictionary dict;

bool LuaInterface::HaltIfError = true;
std::string LuaInterface::lastError;

void FlipScreen()
{
  g_animation.MakeFlip();
}

bool StartPanels()
{
  return g_frameRepo.Begin() && DMADisplay::Start();
}


int ClearScreen()
{
  DMADisplay::Display->clearScreen();
  return 0;
}

int GetAliasByName(std::string aliasName)
{
  return g_frameRepo.getAlias(aliasName);
}

int DrawFace(int i)
{
  g_animation.DrawFrame(g_frameRepo.takeFile(), i);
  g_frameRepo.freeFile();
  return 0;
}
void DrawCurrentFrame()
{
  g_animation.DrawCurrentFrame(g_frameRepo.takeFile());
  g_frameRepo.freeFile();
}

int getInternalButtonStatus()
{
  return digitalRead(EDIT_MODE_PIN);
}

void setPanelBrighteness(uint8_t bright)
{
  DMADisplay::Display->setBrightness(bright);
  DMADisplay::Display->setBrightnessExt(bright);
  return;
}

uint8_t getPanelBrighteness()
{
  return DMADisplay::Display->getBrightnessExt();
}




void gentlySetPanelBrighteness(uint8_t bright, uint8_t rate)
{
  Devices::SetGentlyBrighteness(bright, rate);
  return;
}


bool popPanelAnimation()
{
  return g_animation.PopAnimation();
}


void DrawPixels(std::vector<Pixel> pixels)
{
  DMADisplay::Display->startWrite();
  for (auto &it : pixels ){
    DMADisplay::Display->updateMatrixDMABuffer_2(it.x, it.y, it.r, it.g, it.b);
  }
  DMADisplay::Display->endWrite();
}


void DrawPixel(int16_t x, int16_t y, uint16_t color)
{
  DMADisplay::Display->drawPixel(x, y, color);
}

void DrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  DMADisplay::Display->fillRect(x, y, w, h, color);
}

void DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  DMADisplay::Display->drawRect(x, y, w, h, color);
}

int DrawLine(int16_t x, int16_t y, int16_t x1, int16_t x2, uint16_t color)
{
  DMADisplay::Display->drawLine(x, y, x1, x2, color);
  return 0;
}

int DrawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  DMADisplay::Display->drawCircle(x, y, r, color);
  return 0;
}

int DrawFillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  DMADisplay::Display->fillCircle(x, y, r, color);
  return 0;
}

uint16_t color444(uint8_t r, uint8_t g, uint8_t b)
{
  return DMADisplay::Display->color444(r, g, b);
}

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
  return DMADisplay::Display->color565(r,g,b);
}

uint16_t readLidar()
{
  return Devices::ReadLidar();
}
bool hasLidar()
{
  return Devices::HasLidar();
}

uint32_t readButtonStatus(uint32_t button)
{
  if (button >= MAX_BLE_BUTTONS*MAX_BLE_CLIENTS)
  {
    return 0;
  }
  uint32_t id = button/uint32_t(MAX_BLE_BUTTONS);
  return BleManager::remoteData[id].real_inputButtonsStatus[button];
}

float readAccelerometerX(int device)
{
  if (device >= MAX_BLE_CLIENTS){
    return 0.0f;
  }
  return BleManager::remoteData[device].x * 4.0f / 32768.0f;
}

float readAccelerometerY(int device)
{
  if (device >= MAX_BLE_CLIENTS){
    return 0.0f;
  }
  return BleManager::remoteData[device].y * 4.0f / 32768.0f;
}

float readAccelerometerZ(int device)
{
  if (device >= MAX_BLE_CLIENTS){
    return 0.0f;
  }
  return BleManager::remoteData[device].z * 4.0f / 32768.0f;
}

int readGyroX(int device)
{
  if (device >= MAX_BLE_CLIENTS){
    return 0.0f;
  }
  return BleManager::remoteData[device].ax;
}

int readGyroY(int device)
{
  if (device >= MAX_BLE_CLIENTS){
    return 0.0f;
  }
  return BleManager::remoteData[device].ay;
}

int readGyroZ(int device)
{
  if (device >= MAX_BLE_CLIENTS){
    return 0.0f;
  }
  return BleManager::remoteData[device].az;
}


static int dumpStackToSerial(lua_State *L)
{
  QuickDebug::DumpLua(L);
  return 0;
}

/*static int lua_wrapper_print(lua_State *L)
{
  int n = lua_gettop(L); 
  int i;
  lua_getglobal(L, "tostring");
  for (i = 1; i <= n; i++)
  {
    const char *s;
    size_t l;
    lua_pushvalue(L, -1); 
    lua_pushvalue(L, i); 
    lua_call(L, 1, 1);
    s = lua_tolstring(L, -1, &l); 
    if (s == NULL)
      return luaL_error(L, "'tostring' must return a string to 'print'");
    if (i > 1)
    {
      Logger::Print("\t");
    }
    Logger::Print(s);
    lua_pop(L, 1);
  }
  Logger::Println("");
  return 0;
}*/

void setHaltOnError(bool halt)
{
  LuaInterface::HaltIfError = halt;
}

void powerOn()
{
  digitalWrite(PIN_ENABLE_REGULATOR, HIGH);
}

void setPoweringMode(int mode)
{
  Devices::SetPowerMode((PowerMode)mode);
}


void composeBulkFile()
{
  g_frameRepo.composeBulkFile();
}

void powerOff()
{
  digitalWrite(PIN_ENABLE_REGULATOR, LOW);
}

void LuaInterface::luaCallbackError(const char *errMsg)
{
  Logger::Info("Lua error: %s\n", errMsg);
  
  lastError = errMsg;

  if (LuaInterface::HaltIfError)
  {
    powerOff();
    OledScreen::PrintError(errMsg);
    for (;;)
    {
    }
  }
}

int getConnectedRemoteControls()
{
  return g_remoteControls.getConnectedClients();
}

bool startBLE()
{
  Logger::Info("Starting BLE");
  if (!g_remoteControls.begin())
  {
    OledScreen::CriticalFail("Failed to initialize BLE!");
    for (;;)
    {
    }
  }
  Devices::CalculateMemmoryUsage();
  return true;
}

int isElementIdConnected(int id)
{
  return g_remoteControls.isElementIdConnected(id);
}
void beginScanning()
{
  g_remoteControls.beginScanning();
}
void setMaximumControls(int id)
{
  g_remoteControls.setMaximumControls(id);
}

int acceptTypes(std::string service, std::string charactestistic)
{
  return g_remoteControls.acceptTypes(service, charactestistic);
}

void setAnimation(std::vector<int> frames, int duration, int repeatTimes, bool dropAll, int externalStorageId)
{
  g_animation.SetAnimation(duration, frames, repeatTimes, dropAll, externalStorageId);
}

void setSpeakingFrames(std::vector<int> frames, int duration )
{
  g_animation.SetSpeakAnimation(duration, frames);
}


void setManaged(bool bn)
{
  g_animation.setManaged(bn);
}
bool isManaged()
{
  return g_animation.isManaged();
}

int getCurrentAnimationStorage()
{
  return g_animation.getCurrentAnimationStorage();
}
int getCurrentFace()
{
  return g_animation.getCurrentFace();
}

void DrawPanelFaceToScreen(int x, int y)
{
  OledScreen::DrawPanelFaceToScreen(x, y);
  return;
}

std::vector<uint16_t> decodePng(std::string filename)
{
  int lastRcError = 0;
  size_t x;
  size_t y;
  uint16_t* decodedData = Storage::DecodePNG(filename.c_str(), lastRcError,x, y);
  if (lastRcError != 0){
    OledScreen::CriticalFail("File not found");
    for (;;){}
    return std::vector<uint16_t>();
  }
  if (!decodedData){
    OledScreen::CriticalFail("Failed to allocate");
    for (;;){}
  }
  size_t dataSize = x * y;
  std::vector<uint16_t> res(dataSize);
  for (int i=0;i<dataSize;i++){
    res[i] = decodedData[i];
  }
  //Storage::FreeBuffer(decodedData);
  return res;
}



void DrawDisplayScreen()
{
  OledScreen::display.display();
  return;
}

void DrawSetCursor(int x, int y)
{
  OledScreen::display.setCursor(x, y);
  return;
}
void DrawTextScreen(std::string str)
{
  OledScreen::display.print(str.c_str());
  return;
}

void DrawRectScreen(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  OledScreen::display.drawRect(x, y, w, h, color);
  return;
  
}
void DrawLineScreen(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  OledScreen::display.drawLine(x, y, w, h, color);
  return;
}
void DrawCircleScreen(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  OledScreen::display.drawCircle(x,y,r,color);
  return;
}

void DrawFilledCircleScreen(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  OledScreen::display.fillCircle(x, y, r, color);
  return;
}
void DrawSetFontSize(int16_t x)
{
  OledScreen::display.setTextSize(x);
  return;
}

const char *dictget(std::string str){
  return dict.get(str);
}
void dictdel(std::string str){
  return dict.del(str);
}

void dictset(std::string str, const char * val){
  dict.set(str, val);
}

void dictSave(){
  dict.Save();
}

void dictLoad(){
  dict.Load();
}

void dictFormat(){
  dict.Format();
}


void LuaInterface::RegisterMethods()
{

  m_lua->FuncRegister("dictGet", dictget);
  m_lua->FuncRegister("dictSet", dictset);
  m_lua->FuncRegister("dictDet", dictdel);
  m_lua->FuncRegister("dictSave", dictSave);
  m_lua->FuncRegister("dictLoad", dictLoad);
  m_lua->FuncRegister("dictFormat", dictFormat);

  m_lua->FuncRegister("oledFaceToScreen", DrawPanelFaceToScreen);
  m_lua->FuncRegister("oledCreateIcon", OledScreen::CreateIcon);
  m_lua->FuncRegister("oledDrawIcon", OledScreen::DrawIcon);
  m_lua->FuncRegister("oledDrawBottomBar", OledScreen::DrawBottomBar);
  m_lua->FuncRegister("oledClearScreen", OledScreen::Clear);
  m_lua->FuncRegister("oledDisplay", DrawDisplayScreen);
  m_lua->FuncRegister("oledSetCursor", DrawSetCursor);
  m_lua->FuncRegister("oledSetFontSize", DrawSetFontSize);
  m_lua->FuncRegister("oledDrawText", DrawTextScreen);
  m_lua->FuncRegister("oledDrawRect", DrawRectScreen);
  m_lua->FuncRegister("oledDrawLine", DrawLineScreen);
  m_lua->FuncRegister("oledDrawCircle", DrawCircleScreen);
  //m_lua->FuncRegisterFromObject("oledDrawCircle", &OledScreen::display, &Adafruit_SSD1306::drawCircle); 
  m_lua->FuncRegister("oledDrawCircle", DrawCircleScreen);
  m_lua->FuncRegister("oledDrawFilledCircle", DrawFilledCircleScreen);

  m_lua->FuncRegister("startBLE", startBLE);
  m_lua->FuncRegister("getConnectedRemoteControls", getConnectedRemoteControls);
  m_lua->FuncRegister("isElementIdConnected", isElementIdConnected);
  m_lua->FuncRegister("beginBleScanning", beginScanning);
  m_lua->FuncRegister("setMaximumControls", setMaximumControls);
  m_lua->FuncRegister("acceptBLETypes", acceptTypes);

  m_lua->FuncRegister("panelPowerOn", powerOn);
  m_lua->FuncRegister("panelPowerOff", powerOff);
  m_lua->FuncRegister("setPoweringMode", setPoweringMode);
  m_lua->FuncRegister("waitForPower", Devices::WaitForPower);
  m_lua->FuncRegister("setAutoCheckPowerLevel", Devices::SetAutoCheckPowerLevel);

  m_lua->FuncRegister("setVoltageStopThreshold", Devices::SetVoltageStopThreshold);
  m_lua->FuncRegister("setVoltageStartThreshold", Devices::SetVoltageStartThreshold);


  m_lua->FuncRegister("tone", Devices::BuzzerTone);
  m_lua->FuncRegister("toneDuration", Devices::BuzzerToneDuration);
  m_lua->FuncRegister("noTone", Devices::BuzzerNoTone);
  

  m_lua->FuncRegister("getBatteryVoltage", Sensors::GetBatteryVoltage);
  m_lua->FuncRegister("getAvgBatteryVoltage", Sensors::GetAvgBatteryVoltage);
  m_lua->FuncRegister("setHaltOnError", setHaltOnError);
  m_lua->FuncRegister("i2cScan", Devices::I2CScan);

  m_lua->FuncRegister("getFps", Devices::getFps); 
  m_lua->FuncRegister("getFreePsram", Devices::getFreePsram); 
  m_lua->FuncRegister("getFreeHeap", Devices::getFreeHeap); 
  m_lua->FuncRegister("servoPause", Devices::ServoPause);
  m_lua->FuncRegister("servoResume", Devices::ServoResume); 
  m_lua->FuncRegister("servoMove", Devices::ServoMove);
  m_lua->FuncRegister("hasServo", Devices::HasServo);

  m_lua->FuncRegister("readButtonStatus", readButtonStatus);
  m_lua->FuncRegisterOptional("readAccelerometerX", readAccelerometerX, 0);
  m_lua->FuncRegisterOptional("readAccelerometerY", readAccelerometerY, 0);
  m_lua->FuncRegisterOptional("readAccelerometerZ", readAccelerometerZ, 0);
  m_lua->FuncRegisterOptional("readGyroX", readGyroX, 0);
  m_lua->FuncRegisterOptional("readGyroY", readGyroY, 0);
  m_lua->FuncRegisterOptional("readGyroZ", readGyroZ, 0);

  m_lua->FuncRegister("hasLidar", hasLidar);
  m_lua->FuncRegister("readLidar", readLidar);


  m_lua->FuncRegister("getInternalButtonStatus", getInternalButtonStatus); 
  

  m_lua->FuncRegister("startPanels", StartPanels); 
  m_lua->FuncRegister("flipPanelBuffer", FlipScreen);
  m_lua->FuncRegister("drawPanelRect", DrawRect);
  m_lua->FuncRegister("drawPanelFillRect", DrawFillRect);
  m_lua->FuncRegister("drawPanelPixel", DrawPixel);
  m_lua->FuncRegister("drawPanelPixels", DrawPixels);
  m_lua->FuncRegister("drawPanelLine", DrawLine);
  m_lua->FuncRegister("drawPanelCircle", DrawCircle);
  m_lua->FuncRegister("drawPanelFillCircle", DrawFillCircle);
  m_lua->FuncRegister("clearPanelBuffer", ClearScreen);
  m_lua->FuncRegister("drawPanelFace", DrawFace);
  
  m_lua->FuncRegisterOptional("setPanelAnimation", setAnimation, -1, false, -1, 250);
  m_lua->FuncRegister("popPanelAnimation", popPanelAnimation); 
  m_lua->FuncRegister("setPanelMaxBrighteness", Devices::SetMaxBrighteness);
  m_lua->FuncRegisterOptional("gentlySetPanelBrighteness", gentlySetPanelBrighteness, 4);
  m_lua->FuncRegister("setPanelManaged", setManaged);
  m_lua->FuncRegister("isPanelManaged", isManaged);
  m_lua->FuncRegister("getCurrentAnimationStorage", getCurrentAnimationStorage);
  m_lua->FuncRegister("getPanelCurrentFace", getCurrentFace);
  m_lua->FuncRegister("drawPanelCurrentFrame", DrawCurrentFrame);
  m_lua->FuncRegister("setPanelBrighteness", setPanelBrighteness);
  m_lua->FuncRegister("getPanelBrighteness", getPanelBrighteness);
  m_lua->FuncRegister("setSpeakingFrames", setSpeakingFrames);  
  m_lua->FuncRegisterFromObjectOpt("setRainbowShader", &g_animation, &Animation::setRainbowShader, true); 
  m_lua->FuncRegisterFromObjectOpt("getAnimationStackSize", &g_animation, &Animation::getAnimationStackSize); 


  m_lua->FuncRegister("color565", color565);
  m_lua->FuncRegister("color444", color444);
  m_lua->FuncRegister("millis", millis);
  m_lua->FuncRegister("delay", delay);
  m_lua->FuncRegister("digitalWrite", digitalWrite);
  m_lua->FuncRegister("digitalRead", digitalRead);

  m_lua->FuncRegister("analogRead", analogRead);
  m_lua->FuncRegister("pinMode", pinMode);

  m_lua->FuncRegister("delayMicroseconds", delayMicroseconds);
  m_lua->FuncRegisterRaw("dumpStackToSerial", dumpStackToSerial);

  m_lua->FuncRegister("composeBulkFile", composeBulkFile);
  m_lua->FuncRegister("getFrameAliasByName", GetAliasByName);

  m_lua->FuncRegisterFromObjectOpt("ledsSetBrightness", &g_leds, &LedStrip::setBrightness, (uint8_t)128);
  m_lua->FuncRegisterFromObjectOpt("ledsBegin", &g_leds, &LedStrip::Begin, (uint8_t)128);
  m_lua->FuncRegisterFromObjectOpt("ledsBeginDual", &g_leds, &LedStrip::BeginDual, (uint8_t)128);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentRange", &g_leds, &LedStrip::setSegmentRange, 0);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentBehavior", &g_leds, &LedStrip::setSegmentBehavior, 0, 0, 0, 0);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentTweenBehavior", &g_leds, &LedStrip::setSegmentTweenBehavior, 0, 0, 0, 0);
  m_lua->FuncRegisterFromObjectOpt("ledsSegmentTweenSpeed", &g_leds, &LedStrip::setSegmentTweenSpeed);
  m_lua->FuncRegisterFromObjectOpt("setLedColor", &g_leds, &LedStrip::setLedColor);

  m_lua->FuncRegister("decodePng", decodePng); 

}

void LuaInterface::RegisterConstants()
{
  m_lua->setConstant("VCC_THRESHOLD_HALT", VCC_THRESHOLD_HALT);
  m_lua->setConstant("VCC_THRESHOLD_START", VCC_THRESHOLD_START);
  m_lua->setConstant("OLED_SCREEN_WIDTH", OLED_SCREEN_WIDTH);
  m_lua->setConstant("OLED_SCREEN_HEIGHT", OLED_SCREEN_HEIGHT);
  m_lua->setConstant("PANEL_WIDTH", PANEL_WIDTH);
  m_lua->setConstant("PANEL_HEIGHT", PANEL_HEIGHT);

  m_lua->setConstant("BUTTON_RELEASED", BUTTON_RELEASED);
  m_lua->setConstant("BUTTON_JUST_PRESSED", BUTTON_JUST_PRESSED);
  m_lua->setConstant("BUTTON_PRESSED", BUTTON_PRESSED);
  m_lua->setConstant("BUTTON_JUST_RELEASED", BUTTON_JUST_RELEASED);

  m_lua->setConstant("BEHAVIOR_PRIDE", (int)BEHAVIOR_PRIDE);
  m_lua->setConstant("BEHAVIOR_ROTATE", (int)BEHAVIOR_ROTATE);
  m_lua->setConstant("BEHAVIOR_RANDOM_COLOR", (int)BEHAVIOR_RANDOM_COLOR);
  m_lua->setConstant("BEHAVIOR_FADE_CYCLE", (int)BEHAVIOR_FADE_CYCLE);
  m_lua->setConstant("BEHAVIOR_ROTATE_FADE_CYCLE", (int)BEHAVIOR_ROTATE_FADE_CYCLE);
  m_lua->setConstant("BEHAVIOR_STATIC_RGB", (int)BEHAVIOR_STATIC_RGB);
  m_lua->setConstant("BEHAVIOR_STATIC_HSV", (int)BEHAVIOR_STATIC_HSV);
  m_lua->setConstant("BEHAVIOR_RANDOM_BLINK", (int)BEHAVIOR_RANDOM_BLINK);
  m_lua->setConstant("BEHAVIOR_ICON_X", (int)BEHAVIOR_ICON_X);
  m_lua->setConstant("BEHAVIOR_ICON_I", (int)BEHAVIOR_ICON_I);
  m_lua->setConstant("BEHAVIOR_ICON_V", (int)BEHAVIOR_ICON_V);
  m_lua->setConstant("BEHAVIOR_ROTATE_SINE_V", (int)BEHAVIOR_ROTATE_SINE_V);
  m_lua->setConstant("BEHAVIOR_ROTATE_SINE_S", (int)BEHAVIOR_ROTATE_SINE_S);
  m_lua->setConstant("BEHAVIOR_ROTATE_SINE_H", (int)BEHAVIOR_ROTATE_SINE_H);
  m_lua->setConstant("BEHAVIOR_FADE_IN", (int)BEHAVIOR_FADE_IN);


  m_lua->setConstant("BUTTON_LEFT", (int)0);
  m_lua->setConstant("BUTTON_RIGHT", (int)2);
  m_lua->setConstant("BUTTON_UP", (int)3);
  m_lua->setConstant("BUTTON_CONFIRM", (int)4);
  m_lua->setConstant("BUTTON_DOWN", (int)1);

  char bigBuffMsg[100];
  for (int i=0;i<MAX_BLE_CLIENTS;i++){
    int baseCount = i * MAX_BLE_BUTTONS;
    sprintf(bigBuffMsg, "DEVICE_%d_BUTTON_LEFT", i);
    m_lua->setConstant(bigBuffMsg, baseCount);

    sprintf(bigBuffMsg, "DEVICE_%d_BUTTON_DOWN", i);
    m_lua->setConstant(bigBuffMsg, baseCount+1);

    sprintf(bigBuffMsg, "DEVICE_%d_BUTTON_RIGHT", i);
    m_lua->setConstant(bigBuffMsg, baseCount+2);

    sprintf(bigBuffMsg, "DEVICE_%d_BUTTON_UP", i);
    m_lua->setConstant(bigBuffMsg, baseCount+3);

    sprintf(bigBuffMsg, "DEVICE_%d_BUTTON_CONFIRM", i);
    m_lua->setConstant(bigBuffMsg, baseCount+4);
   
  }

  m_lua->setConstant("MAX_BLE_BUTTONS", (int)MAX_BLE_BUTTONS);
  m_lua->setConstant("MAX_BLE_CLIENTS", (int)MAX_BLE_CLIENTS);
  m_lua->setConstant("SERVO_COUNT", (int)SERVO_COUNT);

  m_lua->setConstant("POWER_MODE_USB_5V", (int)POWER_MODE_USB_5V);
  m_lua->setConstant("POWER_MODE_USB_9V", (int)POWER_MODE_USB_9V);
  m_lua->setConstant("POWER_MODE_BATTERY", (int)POWER_MODE_BATTERY);

  
}

bool LuaInterface::Start()
{

  m_lua = new (LuaWrapper);
  if (!m_lua)
  {
    return false;
  }

  m_lua->SetErrorCallback(LuaInterface::luaCallbackError);

  RegisterMethods();
  RegisterConstants();

  lastError = "";

  return true;
}

bool LuaInterface::DoString(const char *content)
{
  return m_lua->Lua_dostring(content);
}

bool LuaInterface::LoadFile(const char *functionName)
{
  if (luaL_dofile(m_lua->GetState(), functionName))
  {
    const char *error_message = lua_tostring(m_lua->GetState(), -1);
    Serial.printf("Error loading file %s:\n%s", functionName, error_message);
    Logger::Info("Lua error: %s\n", error_message);
    return false;
  }
  return true;
}

bool LuaInterface::CallFunction(const char *functionName)
{
  return m_lua->callLuaFunction(functionName);
}