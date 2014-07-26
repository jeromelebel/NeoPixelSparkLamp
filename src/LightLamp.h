typedef enum {
    LightModeRainbow,
    LightModeStaticColor,
} LightMode;

class Adafruit_NeoPixel;

class LightLamp {
private:
    LightMode lightMode;
    int version;
    Adafruit_NeoPixel *strip;

    void rainbowLoop(void);
    void staticColorLoop(void);
    uint32_t wheel(short wheelPos);
    
public:
    int intensity;
    int rainbowTimer;
    int staticColor;
    char lightModeString[128];
    
    LightLamp();
    int setLightModeString(String args);
    void setLightMode(LightMode mode);
    void setRainbowTimer(int value);
    void setIntensity(unsigned char value);
    void setStaticColor(int color);
    void saveAllVariables(void);
    
    void begin(Adafruit_NeoPixel *strip);
    void loop(void);
    
    static LightMode lightModeFromString(String value);
};
