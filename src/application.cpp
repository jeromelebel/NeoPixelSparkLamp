// This #include statement was automatically added by the Spark IDE.
#include "Adafruit_NeoPixel.h"
#include "application.h"
#include "mdns.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, D2, WS2812B);

typedef enum {
    LightModeRainbow,
    LightModeStaticColor,
} LightMode;

int intensity = 255;
int timer = 60;
int staticColor = 0xFFFFFF;
LightMode lightMode = LightModeRainbow;

static void blinkLED(unsigned int count, unsigned int mydelay)
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

static byte byteFromChar(char character)
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

static byte byteFromHexStringAtIndex(String color, int index)
{
    byte result = 0;
    
    result = byteFromChar(color[index]) * 16 + byteFromChar(color[index + 1]);
    return result;
}

static byte valueWithIntensity(byte value)
{
    float result;
    
    result = (float)value / 255.0 * (float)intensity;
    if (result > 255.0) {
        result = 255.0;
    }
    return result;
}

static int setIntensity(String args)
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
    return 200;
}

static int setLightMode(String args)
{
    if (args == "rainbow") {
        lightMode = LightModeRainbow;
        return 200;
    } else if (args == "staticcolor") {
        lightMode = LightModeStaticColor;
        return 200;
    }
    return 400;
}

static int setTimer(String args)
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
    return 200;
}

static void setStripToColor(uint32_t color)
{
    uint16_t i;
    
    for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

static int setColor(String color)
{
    if (color.length() == 6) {
        staticColor = strip.Color(valueWithIntensity(byteFromHexStringAtIndex(color, 0)),
                                  valueWithIntensity(byteFromHexStringAtIndex(color, 2)),
                                  valueWithIntensity(byteFromHexStringAtIndex(color, 4)));
        return staticColor;
    }
    return 400;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
static uint32_t Wheel(byte WheelPos) {
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
static void rainbow(uint8_t wait)
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

#define NETWORK 0

#if NETWORK
char szIP[64];
bool isStarted;
char szHTML[] = "<html><body><p>Spark Core and MDNS Working :)</p></body></html>";

MDNSResponder mdns;
TCPServer server = TCPServer(80);
#endif

void setup()
{
    strip.begin();
    Serial.begin(115200);
    Serial.println("started!");
    Serial.println(Network.SSID());
    Spark.function("setintensity", setIntensity);
    Spark.function("settimer", setTimer);
    Spark.function("setlightmode", setLightMode);
    Spark.function("setcolor", setColor);

    blinkLED(2, 50);
    
#if NETWORK
    IPAddress addr = Network.localIP();
    uint32_t ip = (addr[0] * 16777216) + (addr[1] * 65536) + (addr[2] * 256) + (addr[3]);
    
    // Begin MDNS
    isStarted = mdns.begin("myspark", ip);
    
    // If started, set the variable to IP address of the Spark Core
    // else set error message
    if(isStarted){
        sprintf(szIP, "%d.%d.%d.%d (%u)", addr[0], addr[1], addr[2], addr[3], ip);
         server.begin();
    }
    else{
        sprintf(szIP, "Error starting MDNS.");
    }
    
    Spark.variable("mdnsstatus", szIP, STRING);
#endif
}

void loop()
{
    static unsigned long lastMDNSUpdate = 0;
    
    switch (lightMode) {
    case LightModeRainbow:
        rainbow(timer);
        break;
    case LightModeStaticColor:
        setStripToColor(staticColor);
        break;
    }
#if NETWORK
    if (isStarted && millis() - lastMDNSUpdate > 2000) {
        mdns.update();
        lastMDNSUpdate = millis();
    }        
    TCPClient client = server.available();
    if (client){
        client.write(szHTML);
        client.flush();
        client.stop();
    }
#endif
}
