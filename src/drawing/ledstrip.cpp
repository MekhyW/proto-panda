#include "drawing/ledstrip.hpp"
#include "tools/logger.hpp"
#include "tools/devices.hpp"
#include "config.hpp"
#include <Arduino.h>

bool LedStrip::BeginDual(uint16_t ledCount, uint16_t secondLedCount, uint8_t maxbrighteness){
    if (ledCount > 0){
        Logger::Info("Starting total of %d leds", ledCount+secondLedCount);
        m_ledAmount = ledCount+secondLedCount;
        m_maxBrighteness = maxbrighteness;
        m_leds = new CRGB[m_ledAmount+secondLedCount+1];
        if (m_leds == nullptr){
            return false;
        }

        FastLED.addLeds<LED_STRIP_TYPE,LED_STRIP_PIN_1,GRB>(m_leds, ledCount).setCorrection(TypicalLEDStrip).setDither(m_maxBrighteness < 255);
        Logger::Info("Led started at addr %d to %d", (int)m_leds, ledCount);
        if (secondLedCount > 0){
            Logger::Info("Second started at addr %d to %d", ((int)m_leds)+ ledCount, secondLedCount);
            FastLED.addLeds<LED_STRIP_TYPE,LED_STRIP_PIN_2,GRB>(m_leds + ledCount, secondLedCount).setCorrection(TypicalLEDStrip).setDither(m_maxBrighteness < 255);
        }

        FastLED.setBrightness(m_maxBrighteness);

        for (int i=0;i<MAX_LED_GROUPS;i++){
            m_groups[i] = LedGroup();
        }

        m_enabled = true;
        setAllColor(CRGB(0,0,0));
        

        FastLED.show(); 
    }
    return true;
}

void LedStrip::setAllColor(CRGB col){
    if (!m_enabled){
        return;
    }
    for (int a=0;a<m_ledAmount;++a){
        m_leds[a] = col;
    }
}

void LedStrip::setAllColorHSV(uint8_t h, uint8_t s, uint8_t v){
    if (!m_enabled){
        return;
    }
    for (int a=0;a<m_ledAmount;++a){
        m_leds[a].setHSV(h,s,v);
    }
}

void LedStrip::Update(){
    if (!m_enabled){
        return;
    }
    for (int i=0;i<MAX_LED_GROUPS;i++){
        
        if (m_groups[i].to > 0){
            m_groups[i].Update(m_leds);
        }
    }
}

void LedStrip::setSegmentColor(int id, int r, int g, int b){
    if (id < 0 || id > 15){
        return;
    }
    for (int x=m_groups[id].from;x<=m_groups[id].to;x++){
        m_leds[x].setRGB(r,g,b);
    }
}

void LedStrip::setBrightness(uint8_t val){
    m_maxBrighteness = val;
    FastLED.setBrightness(m_maxBrighteness);
}

void LedStrip::setLedColor(int id, int r, int g, int b){
    if (!m_enabled){
        return;
    }
    if (id > m_ledAmount){
        return;
    }
    m_leds[id].setRGB(r,g,b);
}

void LedStrip::setSegmentRange(int id, int from, int to){
    if (id < 0 || id > 15){
        return;
    }
    if (to >= m_ledAmount){
        Logger::Info("[WARNING] Setting segment %d with the last led beeing %d which is greater than the maximum leds allowed %d", id, to, m_ledAmount );
        to = m_ledAmount-1;
    }
    if (from < 0){
        Logger::Info("[WARNING] Setting segment %d with the first led beeing %d, lower than 1.", id, from );
        from = 0;
    }
    m_groups[id].from = from;
    m_groups[id].to = to;
    m_groups[id].preallocate();
}

int* LedStrip::getSegmentParameter1(int id){
    if (id < 0 || id > 15){
        return nullptr;
    }
    return &m_groups[id].parameter;
}

int* LedStrip::getSegmentParameter2(int id){
    if (id < 0 || id > 15){
        return nullptr;
    }
    return &m_groups[id].parameter2;
}

int* LedStrip::getSegmentParameter3(int id){
    if (id < 0 || id > 15){
        return nullptr;
    }
    return &m_groups[id].parameter3;
}

void LedStrip::setSegmentBehavior(int id, LedBehavior bh, int parameter, int parameter2, int parameter3,  int parameter4){
    if (id < 0 || id > 15){
        return;
    }
    m_groups[id].val = 0;
    m_groups[id].sum = 0;
    m_groups[id].behavior = bh;
    m_groups[id].parameter = parameter;
    m_groups[id].parameter2 = parameter2;
    m_groups[id].parameter3 = parameter3;
    m_groups[id].parameter4 = parameter4;
}

void LedStrip::setSegmentTweenSpeed(int id, int parameter){
    if (id < 0 || id > 15){
        return;
    }
    m_groups[id].m_tweenRate = parameter;
}
void LedStrip::setSegmentTweenBehavior(int id, LedBehavior bh, int parameter, int parameter2, int parameter3,  int parameter4){
    if (id < 0 || id > 15){
        return;
    }
    m_groups[id].addTween(0.001f, bh, parameter, parameter2 , parameter3 , parameter4);
}

void LedStrip::Display(){
    if (!m_enabled){
        return;
    }
    if (m_ledAmount > 0){
        FastLED.show(); 
    }
}

void LedGroup::preallocate(){
    if (m_tweenBufferSize != to-from+1){
        m_tweenBufferSize = to-from+1;
        if (m_tweenBuffer != nullptr){
           delete [] m_tweenBuffer;
        }
        m_tweenBuffer = new CRGB[to-from+1];
        for (int i=0;i<m_tweenBufferSize;i++){
            m_tweenBuffer[i].setRGB(0,0,0);
        }
    }
}

void LedGroup::addTween(float rate, LedBehavior bh, int a_parameter , int a_parameter2 , int a_parameter3 , int a_parameter4 ){
    m_tweenCycle = 0.0f;
    m_onTween = true;

    m_tweening.from = 0;
    m_tweening.to = to-from;

    m_tweening.behavior = behavior;
    m_tweening.parameter = parameter;
    m_tweening.parameter2 = parameter2;
    m_tweening.parameter3 = parameter3;
    m_tweening.parameter4 = parameter4;
    m_tweening.time = time;
    m_tweening.time2 = time2;
    m_tweening.val = val;
    m_tweening.sum = sum;

    preallocate();


    val = 0;
    sum = 0;
    behavior = bh;
    parameter = a_parameter;
    parameter2 = a_parameter2;
    parameter3 = a_parameter3;
    parameter4 = a_parameter4;
}


void LedGroup::Update(CRGB *leds){
    if (m_onTween){
        m_tweening.Update(m_tweenBuffer);

        m_tweenCycle += (m_tweenRate/255.0f) * Devices::getDeltaTime();

        BaseLedGroup::Update(leds);

        if (m_tweenCycle >= 1.0f){
            m_tweenCycle = 0.0f;
            m_onTween = false;
            return;
        }

        int count = 0;
        for (int x=from;x<=to;x++,count++){
            
            leds[x].r = leds[x].r * m_tweenCycle + m_tweenBuffer[count].r * (1.0f - m_tweenCycle);
            leds[x].g = leds[x].g * m_tweenCycle + m_tweenBuffer[count].g * (1.0f - m_tweenCycle);
            leds[x].b = leds[x].b * m_tweenCycle + m_tweenBuffer[count].b * (1.0f - m_tweenCycle);
  
        }
    }else{
        BaseLedGroup::Update(leds);
    }
}

void BaseLedGroup::Update(CRGB *leds){
    if (to == 0){
        return;
    }
    switch (behavior)
    {
        case BEHAVIOR_PRIDE:
            behavior_pride(leds);
            break;
        case BEHAVIOR_ROTATE:
            behavior_rotate(leds);
            break;
        case BEHAVIOR_ROTATE_FADE_CYCLE:
            behavior_rotateFadeCycle(leds);
            break;
        case BEHAVIOR_ROTATE_SINE_H:
            behavior_sineLoop_H(leds);
            break;
        case BEHAVIOR_ROTATE_SINE_S:
            behavior_sineLoop_S(leds);
            break;
        case BEHAVIOR_ROTATE_SINE_V:
            behavior_sineLoop_V(leds);
            break;
        case BEHAVIOR_RANDOM_BLINK:
            behavior_randomBlink(leds);
            break;
        case BEHAVIOR_RANDOM_COLOR:
            behavior_randomColor(leds);
            break;
        case BEHAVIOR_FADE_CYCLE:
            behavior_fadeCycle(leds);
            break;
        case BEHAVIOR_STATIC_RGB:
            behavior_StaticRgb(leds);
            break;
        case BEHAVIOR_STATIC_HSV:
            behavior_StaticHsv(leds);
            break;
        case BEHAVIOR_ICON_X:
            behavior_iconX(leds);
            break;
        case BEHAVIOR_ICON_I:
            behavior_iconI(leds);
            break;
        case BEHAVIOR_ICON_V:
            behavior_iconV(leds);
            break;
        case BEHAVIOR_FADE_IN:
            behavior_fadeIn(leds);
            break;
        default:
            behavior_black(leds);
            break;
    }
}


void BaseLedGroup::behavior_black(CRGB *leds){
    for (int x=from;x<=to;x++){
        leds[x].setHSV( 0,0,0);
    }
}


void BaseLedGroup::behavior_StaticRgb(CRGB *leds){
    for (int x=from;x<=to;x++){
        leds[x].setRGB(parameter, parameter2, parameter3);
    }
}

void BaseLedGroup::behavior_StaticHsv(CRGB *leds){
    for (int x=from;x<=to;x++){
        leds[x].setHSV(parameter, parameter2, parameter3);
    }
}

void BaseLedGroup::behavior_fadeIn(CRGB *leds){

    if (time <= millis()){
        time = millis()+5+parameter4;
        val += (1 + parameter3);
        if (val > 255){
            val = 255;
        }
    }

    for (int x=from;x<=to;x++){
        leds[x].setHSV(parameter, parameter2, val);
    }
    
}
#define getPosArray(xx, yy) (xx + (6 * yy))

void BaseLedGroup::behavior_iconX(CRGB *leds){
    
    for (int x=from;x<=to;x++){
        leds[x].setHSV(0,0,0);
    }
    
    leds[from + getPosArray(0,0)].setRGB(255, 0, 0);
    leds[from + getPosArray(1,1)].setRGB(255, 0, 0);
    leds[from + getPosArray(2,2)].setRGB(255, 0, 0);
    leds[from + getPosArray(3,3)].setRGB(255, 0, 0);
    leds[from + getPosArray(4,4)].setRGB(255, 0, 0);
    leds[from + getPosArray(5,5)].setRGB(255, 0, 0);


    leds[from + getPosArray(5,0)].setRGB(255, 0, 0);
    leds[from + getPosArray(4,1)].setRGB(255, 0, 0);
    leds[from + getPosArray(3,2)].setRGB(255, 0, 0);
    leds[from + getPosArray(2,3)].setRGB(255, 0, 0);
    leds[from + getPosArray(1,4)].setRGB(255, 0, 0);
    leds[from + getPosArray(0,5)].setRGB(255, 0, 0);
}

void BaseLedGroup::behavior_iconI(CRGB *leds){
    
    for (int x=from;x<=to;x++){
        leds[x].setHSV(0,0,0);
    }
    
    leds[from + getPosArray(5,0)].setRGB(255, 123, 0);
    leds[from + getPosArray(5,1)].setRGB(255, 123, 0);
    leds[from + getPosArray(5,2)].setRGB(255, 123, 0);
    leds[from + getPosArray(5,3)].setRGB(255, 123, 0);
    leds[from + getPosArray(5,4)].setRGB(255, 123, 0);
    leds[from + getPosArray(5,5)].setRGB(255, 123, 0);

    leds[from + getPosArray(4,0)].setRGB(255, 123, 0);
    leds[from + getPosArray(4,5)].setRGB(255, 123, 0);

    leds[from + getPosArray(3,1)].setRGB(255, 123, 0);
    leds[from + getPosArray(3,4)].setRGB(255, 123, 0);

    leds[from + getPosArray(2,2)].setRGB(255, 123, 0);
    leds[from + getPosArray(2,3)].setRGB(255, 123, 0);

}

void BaseLedGroup::behavior_iconV(CRGB *leds){
    
    for (int x=from;x<=to;x++){
        leds[x].setHSV(0,0,0);
    }
    
    leds[from + getPosArray(2,0)].setRGB(0, 255, 0);
    leds[from + getPosArray(3,0)].setRGB(0, 255, 0);
    leds[from + getPosArray(4,0)].setRGB(0, 255, 0);
    leds[from + getPosArray(5,0)].setRGB(0, 255, 0);
    leds[from + getPosArray(5,1)].setRGB(0, 255, 0);
    leds[from + getPosArray(4,2)].setRGB(0, 255, 0);
    leds[from + getPosArray(3,3)].setRGB(0, 255, 0);
    leds[from + getPosArray(2,4)].setRGB(0, 255, 0);
    leds[from + getPosArray(1,5)].setRGB(0, 255, 0);

}

void BaseLedGroup::behavior_fadeCycle(CRGB *leds){
    if (sum == 0){
        sum = parameter2;
    }
    if (time <= millis()){
        time = millis()+50;
        val+=sum;
        if (val > 255){
            val = 255;
            sum = -5-parameter2;
        }
        if (val < parameter3){
            val = parameter3;
            sum = 5+parameter2;
        }
    }
    for (int x=from;x<=to;x++){
        leds[x].setHSV( parameter ,255 , val);
    }
}

void BaseLedGroup::behavior_randomColor(CRGB *leds){
    for (int x=from;x<=to;x++){
        leds[x].setHSV( rand()%255,255,255);
    }
}

void BaseLedGroup::behavior_randomBlink(CRGB *leds){
    if (parameter2 == 0){
        parameter2 = 1;
    }
    if (parameter3 == 0){
        parameter3 = 1;
    }
    if (time <= millis()){
        for (int x=from;x<=to;x++){
            if (rand()%1000 <= 500){
                int selected = (rand()%(parameter2*2) - parameter2);
                leds[x].setHSV( parameter + selected , rand()%255, rand()%parameter3);
            }else{
                leds[x].setHSV( 0,0,0);
            }
        }
        time = millis() + parameter4;
    }
    
}
void BaseLedGroup::behavior_rotate(CRGB *leds){
   
    if (time <= millis()){
        leds[from].setHSV( rand()%255 ,255, rand()%128);
        time = millis()+300;
    }

    if (time2 < millis()){
        int i;
        for (i=to;i>=from+1;--i){
            leds[i] = leds[i-1];
        }
        time2 = millis()+50+parameter4;
    }
    
}
void BaseLedGroup::behavior_sineLoop_H(CRGB *leds){
    
    float lenght = to-from;

    float turns = 4;
    float angleSlice = ((360.0f * turns) / lenght) * (PI / 180.0f);

    float position = 0.0f;

    if (time <= millis()){
        val++;
        time = millis()+5+parameter3;
    }
    position += val;

    for (int x=from;x<=to;x++){        
        leds[x].setHSV( 255 * sin(angleSlice * position) , parameter, parameter2 );
        position++;
    }  
}

void BaseLedGroup::behavior_sineLoop_S(CRGB *leds){
    
    float lenght = to-from;

    float turns = 5;
    float angleSlice = ((360.0f * turns) / lenght) * (PI / 180.0f);

    float position = 0.0f;

    if (time <= millis()){
        val++;
        time = millis()+1+parameter3;
    }
    position += val;

    for (int x=from;x<=to;x++){        
        leds[x].setHSV( parameter , 255 * sin(angleSlice * position), parameter2 );
        position++;
    }  
}

void BaseLedGroup::behavior_sineLoop_V(CRGB *leds){
    
    float lenght = to-from;

    float turns = 2;
    float angleSlice = ((360.0f * turns) / lenght) * (PI / 180.0f);

    float position = 0.0f;

    if (time <= millis()){
        val++;
        time = millis()+5+parameter3;
    }
    position += val;

    for (int x=from;x<=to;x++){        
        leds[x].setHSV( parameter , parameter2, 255 * abs(sin(angleSlice * position)) );
        position++;
    }
    
}

void BaseLedGroup::behavior_rotateFadeCycle(CRGB *leds){
    
    if (time <= millis()){
        val+=sum;

        if (sum == 0){
            sum = 5;
        }

        if (val > 255){
            val = 255;
            sum = -5-parameter2;
        }
        if (val < parameter3){
            val = parameter3;
            sum = 5+parameter2;
        }
        
        int randSat = 100 + rand()%155;

        leds[from].setHSV( parameter ,randSat , val);
        
        time = millis()+30+parameter4;
    }

    if (time2 < millis()){
        int i;
        for (i=to;i>=from+1;--i){
            leds[i] = leds[i-1];
        }
        time2 = millis()+10+parameter4;
    }
    
}

void BaseLedGroup::behavior_pride(CRGB *leds) 
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = from ; i <= to; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
  
    nblend( leds[i], newcolor, 64);
  }
}