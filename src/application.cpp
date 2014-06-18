// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, D2, WS2812B);

typedef enum {
    LightModeRainbow,
    LightModeStaticColor,
} LightMode;

int version = 1;
int intensity = 255;
int timer = 60;
int staticColor = 0xFFFFFF;
LightMode lightMode = LightModeRainbow;


#define CURRENT_VERSION 2
#define VERSION_ADDRESS 0
#define INTENSITY_ADDRESS (VERSION_ADDRESS + sizeof(version))
#define TIMER_ADDRESS (INTENSITY_ADDRESS + sizeof(intensity))
#define STATICCOLOR_ADDRESS (TIMER_ADDRESS + sizeof(timer))
#define LIGHTMODE_ADDRESS (STATICCOLOR_ADDRESS + sizeof(timer))

void rainbow(uint8_t wait);
void blinkLED(unsigned int count, unsigned int mydelay);
int setTimer(String args);
int setIntensity(String args);
int setLightMode(String args);
void setStripToColor(uint32_t color);
byte byteFromChar(char character);
byte byteFromHexStringAtIndex(String color, int index);
int setColor(String color);
byte valueWithIntensity(byte value);
uint32_t Wheel(byte WheelPos);

void blinkLED(unsigned int count, unsigned int mydelay)
{
    int led = D7;
    
    pinMode(led, OUTPUT);
    while (count > 0) {
        digitalWrite(led, HIGH);
        delay(mydelay);               // Wait for 1000mS = 1 second
        digitalWrite(led, LOW); 
        delay(mydelay);               // Wait for 1 second in off mode
        count--;
    }
}

void readBuffer(void *buffer, size_t bufferSize, int address)
{
    size_t ii;
    byte *byteBuffer = (byte *)buffer;
    
    for (ii = 0; ii < bufferSize; ii++) {
        byteBuffer[ii] = EEPROM.read(address);
        address++;
    }
}

void writeBuffer(const void *buffer, size_t bufferSize, int address)
{
    size_t ii;
    const byte *byteBuffer = (const byte *)buffer;
    
    for (ii = 0; ii < bufferSize; ii++) {
        EEPROM.write(address, byteBuffer[ii]);
        address++;
    }
}

void saveAllVariables(void)
{
    writeBuffer(&version, sizeof(version), VERSION_ADDRESS);
    writeBuffer(&intensity, sizeof(intensity), INTENSITY_ADDRESS);
    writeBuffer(&timer, sizeof(timer), TIMER_ADDRESS);
    writeBuffer(&staticColor, sizeof(intensity), STATICCOLOR_ADDRESS);
    writeBuffer(&lightMode, sizeof(lightMode), LIGHTMODE_ADDRESS);
}

void setup()
{
    strip.begin();
    Serial.begin(115200);
    
    readBuffer(&version, sizeof(version), VERSION_ADDRESS);
    if (version == CURRENT_VERSION) {
        readBuffer(&intensity, sizeof(intensity), INTENSITY_ADDRESS);
        readBuffer(&timer, sizeof(timer), TIMER_ADDRESS);
        readBuffer(&staticColor, sizeof(intensity), STATICCOLOR_ADDRESS);
        readBuffer(&lightMode, sizeof(lightMode), LIGHTMODE_ADDRESS);
    } else {
        version = CURRENT_VERSION;
        saveAllVariables();
    }
    
    Serial.println("started!");
    Spark.function("setintensity", setIntensity);
    Spark.function("settimer", setTimer);
    Spark.function("setlightmode", setLightMode);
    Spark.function("setcolor", setColor);
    Spark.function("setcolor", setColor);
}

void loop()
{
    switch (lightMode) {
    case LightModeRainbow:
        rainbow(timer);
        break;
    case LightModeStaticColor:
        setStripToColor(staticColor);
        break;
    }
}

int setTimer(String args)
{
    int value;
    
    value = args.toInt();
    if (value < 0) {
        value = 0;
        return 400;
    } else if (value > 255) {
        value = 255;
        return 400;
    }
    timer = value;
    saveAllVariables();
    return 200;
}

int setIntensity(String args)
{
    int value;
    
    value = args.toInt();
    if (value < 0) {
        value = 0;
        return 400;
    } else if (value > 255) {
        value = 255;
        return 400;
    }
    intensity = value;
    saveAllVariables();
    return 200;
}

int setLightMode(String args)
{
    if (args == "rainbow") {
        lightMode = LightModeRainbow;
        saveAllVariables();
        return 200;
    } else if (args == "staticcolor") {
        lightMode = LightModeStaticColor;
        saveAllVariables();
        return 200;
    }
    return 400;
}

void setStripToColor(uint32_t color)
{
    uint16_t i;
    
    for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

byte byteFromChar(char character)
{
    if (character >= '0' && character <= '9') {
        return character - '0';
    } else if (character >= 'a' && character <= 'f') {
        return character - 'a' + 10;
    } else if (character >= 'A' && character <= 'F') {
        return character - 'A' + 10;
    } else {
        return 15;
    }
}

byte byteFromHexStringAtIndex(String color, int index)
{
    byte result = 0;
    
    result = byteFromChar(color[index]) * 16 + byteFromChar(color[index + 1]);
    return result;
}

int setColor(String color)
{
    if (color.length() == 6) {
        staticColor = strip.Color(valueWithIntensity(byteFromHexStringAtIndex(color, 0)),
                                  valueWithIntensity(byteFromHexStringAtIndex(color, 2)),
                                  valueWithIntensity(byteFromHexStringAtIndex(color, 4)));
        return staticColor;
    }
    return 400;
}

void rainbow(uint8_t wait)
{
    static uint16_t rotation = 0;
    uint16_t i;

    for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((rotation + i) & 255));
    }
    strip.show();
    rotation++;
    if (rotation > 255) {
        rotation = 0;
    }
    delay(wait);
}

byte valueWithIntensity(byte value)
{
    float result;
    
    result = (float)value / 255.0 * (float)intensity;
    if (result > 255.0) {
        result = 255.0;
    }
    return result;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    byte grad;
    byte inverseGrad;
    
    if(WheelPos < 85) {
        grad = valueWithIntensity(WheelPos * 3);
        inverseGrad = valueWithIntensity(255 - WheelPos * 3);
        return strip.Color(grad, inverseGrad, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        grad = valueWithIntensity(WheelPos * 3);
        inverseGrad = valueWithIntensity(255 - WheelPos * 3);
        return strip.Color(inverseGrad, 0, grad);
    } else {
        WheelPos -= 170;
        grad = valueWithIntensity(WheelPos * 3);
        inverseGrad = valueWithIntensity(255 - WheelPos * 3);
        return strip.Color(0, grad, inverseGrad);
    }
}