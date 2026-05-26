#include "drawing/rendering/shader.hpp"
#include "tools/devices.hpp"
#include <math.h>
#include <algorithm>
#include <Arduino.h>

uint32_t ShaderProcessor::FrameId = 0;
uint32_t ShaderProcessor::Time = 0;
uint16_t* ShaderProcessor::Texture = nullptr;

void ShaderProcessor::Hsv2Rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
  // Convert hue to degrees
  float hue = h / 255.0f * 360.0f;

  // Convert saturation and value to percentages
  float saturation = s / 255.0f;
  float value = v / 255.0f;

  // Calculate chroma
  float chroma = value * saturation;

  // Find the hue sector
  float hue_sector = hue / 60.0f;
  int hue_sector_int = (int)hue_sector;

  // Calculate the intermediate value x
  float x = chroma * (1.0f - fabs(fmod(hue_sector, 2.0f) - 1.0f));

  // Calculate the values of r, g, and b
  float r_temp = 0.0f, g_temp = 0.0f, b_temp = 0.0f;
  switch(hue_sector_int) {
    case 0:
      r_temp = chroma;
      g_temp = x;
      b_temp = 0;
      break;
    case 1:
      r_temp = x;
      g_temp = chroma;
      b_temp = 0;
      break;
    case 2:
      r_temp = 0;
      g_temp = chroma;
      b_temp = x;
      break;
    case 3:
      r_temp = 0;
      g_temp = x;
      b_temp = chroma;
      break;
    case 4:
      r_temp = x;
      g_temp = 0;
      b_temp = chroma;
      break;
    case 5:
      r_temp = chroma;
      g_temp = 0;
      b_temp = x;
      break;
  }

  // Calculate the final values of r, g, and b
  float m = value - chroma;
  r = (uint8_t)((r_temp + m) * 255);
  g = (uint8_t)((g_temp + m) * 255);
  b = (uint8_t)((b_temp + m) * 255);
}

void ShaderProcessor::ShaderNone(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType &shdr, float &shaderStrength){
}


void ShaderProcessor::ShaderRainbow(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType &shdr, float &shaderStrength){

    uint8_t rainbowR, rainbowG, rainbowB;

    float gray = (r+g+b)/3.0f;

    Hsv2Rgb((( (ShaderProcessor::FrameId + x) % 64) / 64.0f) * 255, 255, gray, rainbowR, rainbowG, rainbowB);

    if (shaderStrength >= 1.0f) {
        // Full rainbow effect
        r = rainbowR;
        g = rainbowG;
        b = rainbowB;
    } else if (shaderStrength <= 0.0f) {
        // No effect, keep original colors
        // (r, g, b unchanged)
    } else {
        // Blend: result = original * (1 - strength) + rainbow * strength
        r = (uint8_t)(r * (1.0f - shaderStrength) + rainbowR * shaderStrength);
        g = (uint8_t)(g * (1.0f - shaderStrength) + rainbowG * shaderStrength);
        b = (uint8_t)(b * (1.0f - shaderStrength) + rainbowB * shaderStrength);
    }
}

void ShaderProcessor::ShaderFire(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType &shdr, float &shaderStrength){
    
    float time = ShaderProcessor::Time * 0.001f;
    
    // Fire parameters
    float heightFactor = 1.0f - (float)y / 32.0f;  // 0 at top, 1 at bottom
    
    // Add turbulence using multiple sine waves
    float turbulence1 = sin(x * 0.2f + time * 5.0f) * 0.3f;
    float turbulence2 = sin(x * 0.5f - time * 7.5f) * 0.2f;
    float turbulence3 = cos(y * 0.3f + time * 10.0f) * 0.15f;
    
    float fireIntensity = heightFactor + turbulence1 + turbulence2 + turbulence3;
    
    // Flickering effect
    float flicker = 0.7f + 0.3f * sin(time * 20.0f + x * 0.5f);
    fireIntensity *= flicker;
    
    fireIntensity = std::clamp(fireIntensity, 0.0f, 1.0f);
    
    uint8_t fireR, fireG, fireB;
    
    if (fireIntensity > 0.8f) {
        // White-hot core
        float t = (fireIntensity - 0.8f) / 0.2f;
        fireR = 255;
        fireG = 255;
        fireB = (uint8_t)(255 * (1.0f - t));
    } else if (fireIntensity > 0.6f) {
        // Yellow/orange
        float t = (fireIntensity - 0.6f) / 0.2f;
        fireR = 255;
        fireG = (uint8_t)(255 * t);
        fireB = 0;
    } else if (fireIntensity > 0.3f) {
        // Orange/red
        float t = (fireIntensity - 0.3f) / 0.3f;
        fireR = 255;
        fireG = (uint8_t)(255 * t * 0.6f);
        fireB = 0;
    } else {
        // Dark red/black
        float t = fireIntensity / 0.3f;
        fireR = (uint8_t)(255 * t * 0.5f);
        fireG = 0;
        fireB = 0;
    }
    
    // Blue flame at the base (gas fire effect)
    if (y > 28 && fireIntensity > 0.7f) {
        fireB = (uint8_t)std::min(255, fireB + 80);
        fireG = (uint8_t)std::min(255, fireG + 40);
    }

    float brightness = std::max(r,std::max(g,b)) / 255.0f;
    
    float rr = (fireR * brightness);
    float gg = (fireG * brightness);
    float bb = (fireB * brightness);
    
    // Blend with original color based on shader strength
    float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
    r = (uint8_t)(r * (1.0f - strength) + rr * strength);
    g = (uint8_t)(g * (1.0f - strength) + gg * strength);
    b = (uint8_t)(b * (1.0f - strength) + bb * strength);
}

void ShaderProcessor::ShaderTexture(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType &shdr, float &shaderStrength){
    
    // Simple UV mapping - map screen coordinates directly to texture
    if (ShaderProcessor::Texture != nullptr) {
        uint16_t textureColor = ShaderProcessor::Texture[y * PANEL_WIDTH + x];
        uint8_t textureR, textureG, textureB;
        Devices::Display->color565to888(textureColor, textureR, textureG, textureB);
        
        // Blend with original color based on shaderStrength
        float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
        r = (uint8_t)(r * (1.0f - strength) + textureR * strength);
        g = (uint8_t)(g * (1.0f - strength) + textureG * strength);
        b = (uint8_t)(b * (1.0f - strength) + textureB * strength);
    }
}


uint8_t colors_r[5] = {51, 255, 190, 255, 51};
uint8_t colors_g[5] = {255, 153, 190, 153, 255};
uint8_t colors_b[5] = {255, 190, 190, 190, 255};
    

void ShaderProcessor::ShaderTrans(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType &shdr, float &shaderStrength){
    //Fuck transphobes
    float time = ShaderProcessor::Time * 0.001f;
    
    const int screenHeight = 32;
    const int transitionWidth = 3;  
    const int bandHeight = (screenHeight - (4 * transitionWidth)) / 5; 

    float scrollSpeed = 10.0f;  // Pixels per second
    int scrollOffset = (int)(time * scrollSpeed) % screenHeight;
    int yPos = (y + scrollOffset) % screenHeight;
    
    int band = 0;
    float blend = 0.0f;

    
    if (yPos < bandHeight) {
        band = 0;
        blend = 0.0f;
    } 
    else if (yPos < bandHeight + transitionWidth) {
        band = 0;
        blend = (float)(yPos - bandHeight) / transitionWidth;
    }
    else if (yPos < bandHeight * 2 + transitionWidth) {
        // Band 1
        band = 1;
        blend = 0.0f;
    }
    else if (yPos < bandHeight * 2 + transitionWidth * 2) {
        band = 1;
        blend = (float)(yPos - (bandHeight * 2 + transitionWidth)) / transitionWidth;
    }
    else if (yPos < bandHeight * 3 + transitionWidth * 2) {
        band = 2;
        blend = 0.0f;
    }
    else if (yPos < bandHeight * 3 + transitionWidth * 3) {
        band = 2;
        blend = (float)(yPos - (bandHeight * 3 + transitionWidth * 2)) / transitionWidth;
    }
    else if (yPos < bandHeight * 4 + transitionWidth * 3) {
        band = 3;
        blend = 0.0f;
    }
    else if (yPos < bandHeight * 4 + transitionWidth * 4) {
        band = 3;
        blend = (float)(yPos - (bandHeight * 4 + transitionWidth * 3)) / transitionWidth;
    }
    else {
        band = 4;
        blend = 0.0f;
    }
    
    uint8_t targetR, targetG, targetB;
    
    if (blend > 0.0f && band < 4) {
        targetR = (uint8_t)(colors_r[band] * (1.0f - blend) + colors_r[band + 1] * blend);
        targetG = (uint8_t)(colors_g[band] * (1.0f - blend) + colors_g[band + 1] * blend);
        targetB = (uint8_t)(colors_b[band] * (1.0f - blend) + colors_b[band + 1] * blend);
    } else {
        targetR = colors_r[band];
        targetG = colors_g[band];
        targetB = colors_b[band];
    }
    
    float brightness = std::max(r,std::max(g,b)) / 255.0f;  

    // Modulate the target color by the original brightness
    float rr = (targetR * brightness);
    float gg = (targetG * brightness);
    float bb = (targetB * brightness);
    
    // Blend with original color based on shader strength
    float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
    r = (uint8_t)(r * (1.0f - strength) + rr * strength);
    g = (uint8_t)(g * (1.0f - strength) + gg * strength);
    b = (uint8_t)(b * (1.0f - strength) + bb * strength);
}

void ShaderProcessor::UpdateColorByShader(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType &shdr, float &shaderStrength){
    switch (shdr)
    {
    case SHADER_NONE:
        ShaderNone(x, y, r, g, b, shdr, shaderStrength);
        break;
    
    case SHADER_RAINBOW:
        ShaderRainbow(x, y, r, g, b, shdr, shaderStrength);
        break;

    case SHADER_FIRE:
        ShaderFire(x, y, r, g, b, shdr, shaderStrength);  
        break;
    
    case SHADER_TEXTURE:
        ShaderTexture(x, y, r, g, b, shdr, shaderStrength);  
        break;
    
    case SHADER_TRANS:
        ShaderTrans(x, y, r, g, b, shdr, shaderStrength);  
        break;
    
    default:
        break;
    }
}


void ShaderProcessor::IncrFrame(){
    ShaderProcessor::FrameId++;
    ShaderProcessor::Time = millis();
}