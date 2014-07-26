// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "Adafruit_NeoPixel.h"
#include "LightLamp.h"
#include "mdns.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, D2, WS2812B);

MDNSResponder mdns;
int mdnsStatus = 0;
char ipAddress[24];
char compiledVersion[64];
TCPServer server = TCPServer(80);
LightLamp lamp;

void rainbow(uint8_t wait);
void blinkLED(unsigned int count, unsigned int mydelay);
int setTimer(String args);
int setIntensity(String args);
int setLightMode(String args);
void setStripToColor(uint32_t color);
byte byteFromChar(char character);
byte byteFromHexStringAtIndex(String color, int index);
int setColor(String color);
uint32_t Wheel(short WheelPos);

void setup()
{
    IPAddress myIp = Network.localIP();
    
    strip.begin();
    Serial.begin(115200);
    
    mdnsStatus = mdns.begin("boule", myIp);
    sprintf(ipAddress, "%d.%d.%d.%d", myIp[0], myIp[1], myIp[2], myIp[3]);
    sprintf(compiledVersion, "%s-%s", __DATE__, __TIME__);
    
    lamp.begin(&strip);
    Serial.println("Start!!!");
    Spark.function("setintensity", setIntensity);
    Spark.function("settimer", setTimer);
    Spark.function("setlightmode", setLightMode);
    Spark.function("setcolor", setColor);
    
    Spark.variable("mdns", &mdnsStatus, INT);
    Spark.variable("intensity", &lamp.intensity, INT);
    Spark.variable("timer", &lamp.rainbowTimer, INT);
    Spark.variable("lightmode", lamp.lightModeString, STRING);
    Spark.variable("ipaddress", ipAddress, STRING);
    Spark.variable("version", compiledVersion, STRING);
    server.begin();
}

void loop()
{
    TCPClient client = server.available();
    if (client) {
        client.write("prout");
        client.flush();
        client.stop();
    }
    mdns.update();
    lamp.loop();
}

int setTimer(String args)
{
    int value;
    
    value = args.toInt();
    if (value < 0) {
        value = 0;
        return -1;
    }
    lamp.setRainbowTimer(value);
    return value;
}

int setIntensity(String args)
{
    int value;
    
    value = args.toInt();
    if (value < 0) {
        return -1;
    } else if (value > 255) {
        return -1;
    }
    lamp.setIntensity(value);
    return value;
}

int setLightMode(String args)
{
    if (args == "rainbow") {
        lamp.setLightMode(LightModeRainbow);
        return LightModeRainbow;
    } else if (args == "staticcolor") {
        lamp.setLightMode(LightModeStaticColor);
        return LightModeStaticColor;
    }
    return -1;
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
        int value = strip.Color(byteFromHexStringAtIndex(color, 0),
                                  byteFromHexStringAtIndex(color, 2),
                                  byteFromHexStringAtIndex(color, 4));
        lamp.setStaticColor(value);
        return value;
    }
    return -1;
}
