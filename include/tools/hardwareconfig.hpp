#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ArduinoJson.h>

class BaseDisplay;
class HardwareConfig{
    public:
        static bool LoadConfigs();
        static bool StartDmaDisplay();

        static inline int PanelWidth(){
            return HardwarePanelWidth;
        }

        static inline int PanelHeight(){
            return HardwarePanelHeight;
        }
        
    private:
        static HUB75_I2S_CFG panelConfig;
        static void loadAndParseDisplay(JsonObject doc);
        static void loadHub75AndStart(JsonObject doc);
        static void loadWS2812BAndStart(JsonObject doc);
        static void loadMax7219AndStart(JsonObject doc);
        static void loadServosAndStart(JsonObject doc);
        static void loadDefaults();
        static int checkInvalidPin(int pin);
        static void loadViews(JsonObject ws2812b, BaseDisplay* display, uint16_t defaultWidth, uint16_t defaultHeight);

        static int HardwarePanelWidth;
        static int HardwarePanelHeight;

};


