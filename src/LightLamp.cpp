#include "application.h"
#include "LightLamp.h"
#include "Adafruit_NeoPixel.h"

#define CURRENT_VERSION 2
#define VERSION_ADDRESS 0
#define INTENSITY_ADDRESS (VERSION_ADDRESS + sizeof(version))
#define TIMER_ADDRESS (INTENSITY_ADDRESS + sizeof(intensity))
#define STATICCOLOR_ADDRESS (TIMER_ADDRESS + sizeof(rainbowTimer))
#define LIGHTMODE_ADDRESS (STATICCOLOR_ADDRESS + sizeof(staticColor))

static const char *lightModeStringFromMode(LightMode mode)
{
    switch(mode) {
        case LightModeRainbow:
            return "rainbow";
        case LightModeStaticColor:
            return "staticcolor";
        default:
            return "unknown";
    }
}

LightMode LightLamp::lightModeFromString(String string)
{
    if (string == "rainbow") {
        return LightModeRainbow;
    } else if (string == "staticcolor") {
        return LightModeStaticColor;
    } else {
        return (LightMode)-1;
    }
}

static void readBuffer(void *buffer, size_t bufferSize, int address)
{
    size_t ii;
    byte *byteBuffer = (byte *)buffer;
    
    for (ii = 0; ii < bufferSize; ii++) {
        byteBuffer[ii] = EEPROM.read(address);
        address++;
    }
}

static void writeBuffer(const void *buffer, size_t bufferSize, int address)
{
    size_t ii;
    const byte *byteBuffer = (const byte *)buffer;
    
    for (ii = 0; ii < bufferSize; ii++) {
        EEPROM.write(address, byteBuffer[ii]);
        address++;
    }
}

LightLamp::LightLamp()
{
    lightMode = LightModeRainbow;
    lightModeString[0] = 0;
    version = 1;
    intensity = 255;
    rainbowTimer = 60;
    staticColor = 0xFFFFFF;
}

void LightLamp::setLightMode(LightMode mode)
{
    const char *string;
    
    lightMode = mode;
    saveAllVariables();
    string = lightModeStringFromMode(mode);
    strcpy(lightModeString, string);
}

int LightLamp::setLightModeString(String args)
{
    LightMode newMode = LightLamp::lightModeFromString(args);
    
    if (newMode != -1) {
        setLightMode(newMode);
        return 200;
    } else {
        return 400;
    }
}

void LightLamp::setRainbowTimer(int value)
{
    rainbowTimer = value;
    saveAllVariables();
}

void LightLamp::setIntensity(unsigned char value)
{
    intensity = value;
    saveAllVariables();
}

void LightLamp::setStaticColor(int color)
{
}

void LightLamp::staticColorLoop(void)
{
    uint16_t i;
    
    for(i=0; i<strip->numPixels(); i++) {
        strip->setPixelColor(i, staticColor);
    }
    strip->show();
}

void LightLamp::rainbowLoop(void)
{
    static short rotation = 0;
    short i;

    for(i=0; i<strip->numPixels(); i++) {
        strip->setPixelColor(i, wheel((rotation + i) % 0x2FF));
    }
    strip->show();
    rotation++;
    if (rotation > 0x2FF) {
        rotation = 0;
    }
    delay(rainbowTimer);
}

static byte valueWithIntensity(byte value, int intensity)
{
    float result;
    
    result = (float)value / 255.0 * (float)intensity;
    if (result > 255.0) {
        result = 255.0;
    }
    return result;
}

uint32_t LightLamp::wheel(short wheelPos)
{
    byte grad;
    byte inverseGrad;
    
    if(wheelPos > 0x1FF) {
        wheelPos = wheelPos & 0xFF;
        grad = valueWithIntensity(wheelPos, intensity);
        inverseGrad = valueWithIntensity(255 - wheelPos, intensity);
        return strip->Color(inverseGrad, 0, grad);
    } else if(wheelPos > 0xFF) {
        wheelPos = wheelPos & 0xFF;
        grad = valueWithIntensity(wheelPos, intensity);
        inverseGrad = valueWithIntensity(255 - wheelPos, intensity);
        return strip->Color(grad, inverseGrad, 0);
    } else {
        wheelPos = wheelPos & 0xFF;
        grad = valueWithIntensity(wheelPos, intensity);
        inverseGrad = valueWithIntensity(255 - wheelPos, intensity);
        return strip->Color(0, grad, inverseGrad);
    }
}

void LightLamp::saveAllVariables(void)
{
    writeBuffer(&version, sizeof(version), VERSION_ADDRESS);
    writeBuffer(&intensity, sizeof(intensity), INTENSITY_ADDRESS);
    writeBuffer(&rainbowTimer, sizeof(rainbowTimer), TIMER_ADDRESS);
    writeBuffer(&staticColor, sizeof(intensity), STATICCOLOR_ADDRESS);
    writeBuffer(&lightMode, sizeof(lightMode), LIGHTMODE_ADDRESS);
}

void LightLamp::begin(Adafruit_NeoPixel *value)
{
    strip = value;
    readBuffer(&version, sizeof(version), VERSION_ADDRESS);
    if (version == CURRENT_VERSION) {
        LightMode mode;
        
        readBuffer(&intensity, sizeof(intensity), INTENSITY_ADDRESS);
        readBuffer(&rainbowTimer, sizeof(rainbowTimer), TIMER_ADDRESS);
        readBuffer(&staticColor, sizeof(intensity), STATICCOLOR_ADDRESS);
        readBuffer(&mode, sizeof(lightMode), LIGHTMODE_ADDRESS);
        setLightMode(mode);
    } else {
        version = CURRENT_VERSION;
        saveAllVariables();
    }
}

void LightLamp::loop(void)
{
    switch (lightMode) {
    case LightModeRainbow:
        rainbowLoop();
        break;
    case LightModeStaticColor:
        staticColorLoop();
        break;
    }
}
