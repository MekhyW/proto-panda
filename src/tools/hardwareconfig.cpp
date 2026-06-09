#include "tools/hardwareconfig.hpp"
#include "tools/oledscreen.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "tools/psrammap.hpp"
#include "tools/logger.hpp"
#include "lua/luainterface.hpp"
#include "drawing/ledstrip.hpp"

#if PANDA_SD_MODE == 1
#include <SD.h>
#elif PANDA_SD_MODE == 2
#include <SD_MMC.h>
#else
#error "NO SD_MODE Mode defined (set PANDA_SD_MODE to 1 for SD or 2 for SD_MMC)"
#endif

extern LedStrip g_leds;


int HardwareConfig::HardwarePanelWidth = 64;
int HardwareConfig::HardwarePanelHeight = 32;

const uint8_t invalidPins[] = {
    I2C_SDA,
    I2C_SLC,
    SPI_CS,
    SPI_MOSI,
    SPI_MISO,
    SPI_SCK,
    EDIT_MODE_PIN,
    #ifdef PIN_ENABLE_REGULATOR
        PIN_ENABLE_REGULATOR,
    #endif
    #ifdef USE_BUZZER
        BUZZER_PIN,
    #endif
    #ifdef USE_PIN_BATTERY_IN
        PIN_USB_BATTERY_IN,
    #endif
};

HUB75_I2S_CFG HardwareConfig::panelConfig(
    64,  
    32,   
    1 

);

void HardwareConfig::loadDefaults(){
    panelConfig.double_buff = true; // Turn of double buffer
    panelConfig.clkphase = false;
    panelConfig.gpio.r1 = DMA_GPIO_R1;
    panelConfig.gpio.g1 = DMA_GPIO_G1;
    panelConfig.gpio.b1 = DMA_GPIO_B1;
    panelConfig.gpio.r2 = DMA_GPIO_R2;
    panelConfig.gpio.g2 = DMA_GPIO_G2;
    panelConfig.gpio.b2 = DMA_GPIO_B2;
    panelConfig.gpio.a = DMA_GPIO_A;
    panelConfig.gpio.b = DMA_GPIO_B;
    panelConfig.gpio.c = DMA_GPIO_C;
    panelConfig.gpio.d = DMA_GPIO_D;
    panelConfig.gpio.lat = DMA_GPIO_LAT;
    panelConfig.gpio.oe = DMA_GPIO_OE;
    panelConfig.gpio.clk = DMA_GPIO_CLK;
    panelConfig.setPixelColorDepthBits(12);
    panelConfig.i2sspeed = HUB75_I2S_CFG::HZ_20M;
    HardwarePanelWidth = 64;
    HardwarePanelHeight = 32;
}

int HardwareConfig::checkInvalidPin(int pin){
    for (int i=0;i<sizeof(invalidPins);i++){
        if (invalidPins[i] == pin){
            char errMsg[100];
            sprintf(errMsg, "Allocated pin %d but its restricted", pin);
            OledScreen::CriticalFail(errMsg);
        }
    }
    return pin;
}

void HardwareConfig::loadServosAndStart(JsonObject servos){
    if (!servos["enabled"]) {
        return;
    }
    
    if (!servos["pins"].is<JsonArray>()) {
        return;
    }
    
    JsonArray pinsArray = servos["pins"].as<JsonArray>();
    
    if (pinsArray.size() == 0) {
        return;
    }
    
    std::vector<int> pins;
    for (JsonVariant pin : pinsArray) {
        if (pin.is<int>()) {
            pins.push_back(checkInvalidPin(pin.as<int>()));
        }
    }
    
    if (pins.size() > 0) {
        Devices::StartServos(pins);
    }
}

void HardwareConfig::loadAndParseDisplay(JsonObject displayInfo){
    if (!displayInfo.containsKey("type")){
        OledScreen::CriticalFail("Display dont have a type");
        return;
    }

    if (!displayInfo["enabled"]) {
        Devices::Display = new EmptyDisplay();
        return;
    }

    std::string displayType = displayInfo["type"].as<const char*>();
    if (displayType == "hub75"){
        loadHub75AndStart(displayInfo["hub75"]);
    }else if (displayType == "max7219"){
        loadMax7219AndStart(displayInfo["max7219"]);
    }else if (displayType == "ws2812b"){
        loadWS2812BAndStart(displayInfo["ws2812b"]);
    }else{
        OledScreen::CriticalFail("Display type in hardware.json is not supported");
    }
    if (!Devices::Display){
        Devices::Display = new EmptyDisplay();
    }

    if (displayInfo.containsKey("mirrorOtherHalf")) Devices::Display->mirrorHalf = checkInvalidPin(displayInfo["mirrorOtherHalf"]);
}


void HardwareConfig::loadWS2812BAndStart(JsonObject ws2812b){

    uint16_t width,height,brightness;
    if (ws2812b.containsKey("width")){
        width = ws2812b["width"];
    }else{
        OledScreen::CriticalFail("Missing 'width' in display");
    }
    
    if (ws2812b.containsKey("height")) {
        height = ws2812b["height"];
    }else{
        OledScreen::CriticalFail("Missing 'height' in display");
    }
    if (ws2812b.containsKey("brightness")) {
        brightness = ws2812b["brightness"];
    }else{
        OledScreen::CriticalFail("Missing 'brightness' in display");
    }

    CRGB *leds = g_leds.BeginScreen(width, height, brightness);

    Devices::Display = new WS2812BDisplay(width, height, leds);
    if (Devices::Display == nullptr){
        Logger::Info("Failed to start WS2812BD display");
        return;
    }

    Devices::Display->begin();

}

void HardwareConfig::loadMax7219AndStart(JsonObject max7219){
    uint32_t panels = 2;
    int csPin;
    int dataInPin;
    int clockPin;

    if (!max7219.containsKey("pins")){
        OledScreen::CriticalFail("Missing 'pins' in max7219 display");
    }

    JsonObject pins = max7219["pins"];

    if (pins.containsKey("cs")){
        csPin = pins["cs"];
    }else{
        OledScreen::CriticalFail("Missing 'cs' in display");
    }

    if (pins.containsKey("din")){
        dataInPin = pins["din"];
    }else{
        OledScreen::CriticalFail("Missing 'din' in display");
    }

    if (pins.containsKey("clk")){
        clockPin = pins["clk"];
    }else{
        OledScreen::CriticalFail("Missing 'clk' in display");
    }

    if (max7219.containsKey("panels")){
        panels = max7219["panels"];
    }else{
        OledScreen::CriticalFail("Missing 'panels' in display");
    }
    
   
    if (Devices::Display != nullptr){
        Logger::Info("DMA display is already started.");
        return;
    }

    //Todo check avaliable ram
    Devices::Display = new MAX7219Display(panels, csPin, dataInPin, clockPin);
    if (Devices::Display == nullptr){
        Logger::Info("Failed to start DMA display");
        return;
    }

    Devices::Display->begin();
    Devices::Display->setBrightness8(128);
    Devices::Display->clearScreen();
    Devices::Display->flipDma();
    Logger::Info("Started max7219 display!");
}



void HardwareConfig::loadHub75AndStart(JsonObject hub75){
    if (!hub75.containsKey("pins")){
        OledScreen::CriticalFail("Missing 'pins' in hub75 display");
    }

    JsonObject pins = hub75["pins"];

    if (pins.containsKey("dma_r1")) panelConfig.gpio.r1 = checkInvalidPin(pins["dma_r1"]);
    if (pins.containsKey("dma_g1")) panelConfig.gpio.g1 = checkInvalidPin(pins["dma_g1"]);
    if (pins.containsKey("dma_b1")) panelConfig.gpio.b1 = checkInvalidPin(pins["dma_b1"]);
    if (pins.containsKey("dma_r2")) panelConfig.gpio.r2 = checkInvalidPin(pins["dma_r2"]);
    if (pins.containsKey("dma_g2")) panelConfig.gpio.g2 = checkInvalidPin(pins["dma_g2"]);
    if (pins.containsKey("dma_b2")) panelConfig.gpio.b2 = checkInvalidPin(pins["dma_b2"]);
    if (pins.containsKey("dma_a")) panelConfig.gpio.a = checkInvalidPin(pins["dma_a"]);
    if (pins.containsKey("dma_b")) panelConfig.gpio.b = checkInvalidPin(pins["dma_b"]);
    if (pins.containsKey("dma_c")) panelConfig.gpio.c = checkInvalidPin(pins["dma_c"]);
    if (pins.containsKey("dma_d")) panelConfig.gpio.d = checkInvalidPin(pins["dma_d"]);
    if (pins.containsKey("dma_lat")) panelConfig.gpio.lat = checkInvalidPin(pins["dma_lat"]);
    if (pins.containsKey("dma_oe")) panelConfig.gpio.oe = checkInvalidPin(pins["dma_oe"]);
    if (pins.containsKey("dma_clk")) panelConfig.gpio.clk = checkInvalidPin(pins["dma_clk"]);


    if (hub75.containsKey("width")){
        panelConfig.mx_width = hub75["width"];
    }else{
        OledScreen::CriticalFail("Missing 'width' in display");
    }
    
    if (hub75.containsKey("height")) {
        panelConfig.mx_height = hub75["height"];
    }else{
        OledScreen::CriticalFail("Missing 'height' in display");
    }

    if (hub75.containsKey("chain_length")) {
        panelConfig.chain_length = hub75["chain_length"];
    }else{
        OledScreen::CriticalFail("Missing 'chain_length' in display");
    }
        
    if (hub75.containsKey("colordepth")) {
        panelConfig.setPixelColorDepthBits(hub75["colordepth"]);
    }

    StartDmaDisplay();
    return;
}

bool HardwareConfig::LoadConfigs(){
    loadDefaults();

    File conf = PANDA_SD.open( "/hardware.json" );
    if( !conf ) {
        OledScreen::CriticalFail("Can't open hardware.jsonn");
        for(;;){}
        return false;
    }

    SpiRamAllocator allocator;
    JsonDocument  hardwareConfigJson(&allocator);
    auto err = deserializeJson( hardwareConfigJson, conf );
    conf.close();
    if( err ) {
       char miniHBuffer[1000];
       sprintf(miniHBuffer, "hardware.json.json:\n%s", err.c_str());
       OledScreen::CriticalFail(miniHBuffer);
       for(;;){}
       return false;
    }

    if (hardwareConfigJson.containsKey("canvas_width")) {
        HardwareConfig::HardwarePanelWidth = hardwareConfigJson["canvas_width"].as<int>();
    }
    if (hardwareConfigJson.containsKey("canvas_height")) {
        HardwareConfig::HardwarePanelHeight = hardwareConfigJson["canvas_height"].as<int>();
    }

    Logger::Info("Canvas size will be %dx%d", HardwareConfig::HardwarePanelWidth, HardwareConfig::HardwarePanelHeight);

    if (hardwareConfigJson.containsKey("display")) {
        Logger::Info("Loading display info");
        loadAndParseDisplay(hardwareConfigJson["display"]);
    }else{
        Devices::Display = new EmptyDisplay();
        Logger::Info("NO DISPLAY DEFINED!");
    }

    if (hardwareConfigJson.containsKey("servos")) {
        Logger::Info("Loading servos info");
        loadServosAndStart(hardwareConfigJson["servos"]);
    }

    hardwareConfigJson.clear();
    return true;
}

bool HardwareConfig::StartDmaDisplay(){
    if (Devices::Display != nullptr){
        Logger::Info("DMA display is already started.");
        return false;
    }
    //Todo check avaliable ram
    Devices::Display = new Hub75Display(panelConfig);
    if (Devices::Display == nullptr){
        Logger::Info("Failed to start DMA display");
        return false;
    }

    Devices::Display->begin();
    Devices::Display->setBrightness8(1);
    Devices::Display->clearScreen();
    Devices::Display->flipDma();
    Devices::Display->clearScreen();
    Devices::Display->flipDma();
    Logger::Info("DMA display initialized!");
    Devices::CalculateMemmoryUsageDifference("Dma display");

    return true;
}
