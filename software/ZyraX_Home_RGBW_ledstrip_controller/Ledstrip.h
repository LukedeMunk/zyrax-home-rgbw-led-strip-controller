/******************************************************************************/
/*
 * File:    Ledstrip.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * Class:   Ledstrip
 * 
 * Brief:   Class for an addressable RGB(W) LED strip. Handles animation modes,
 *          configuration and pixel based coloring. Tested LED strip drivers:
 *          WS2801, WS2812B, SK6812.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef AddressableLedstrip_H
#define AddressableLedstrip_H
//#define FASTLED_ESP32_USE_CLOCKLESS_SPI
//#define FASTLED_ALL_PINS_HARDWARE_SPI

#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object

#include "SPI.h"                                                                //For sending colors to LED strip over SPI
#include "FastLED.h"                                                            //For sending colors to LED strip over SPI
#include "math.h"
#include "stdlib.h"                                                             //For the NULL keyword
#include "MemoryManager.h"                                                      //For managing files on the SD card
#include "ArduinoJson.h"                                                        //For JSON functionality
#include "FastLED_RGBW.h"                                                       //For RGBW LED support
#include "Preferences.h"                                                        //For non-volatile memory functionality
#include "Configuration.h"                                                      //For configuration variables and global constants
#include "Logger.h"                                                             //For printing and saving logs


#define CORE_NUMBER             1
#define PRIORITY                2


class Ledstrip {
  public:
    Ledstrip();
    void initialize();
    
    /* Direct functions */
    void setPower(bool state, bool startMode = true);
    void doorHandler(bool state);
    void setPowerAnimation(uint8_t animation);
    void setPixelAddressing(String addressesJson, uint16_t numberOfLeds);
    
    /* Modes */
    void setMode(uint8_t mode);
    void configureMode(uint8_t mode, ModeParameters parameters, bool save = true);
    
    void drawPixels(CRGB leds[]);
    void color();
    void fade();
    void gradient();
    void blink();
    void scan();
    void theater();
    void sine();
    void bouncingBalls();
    void dissolve();
    void sparkle();
    void fireworks();
    void fire();
    void sweep();
    void colorTwinkels();
    void meteorRain();
    void colorWaves();
    void modeTemplate1();
    void modeTemplate2();
    void modeTemplate3();
    void modeTemplate4();
    void modeTemplate5();
    void modeTemplate6();
    void modeTemplate7();
    void modeTemplate8();
    void modeTemplate9();
    void modeTemplate10();

    void systemPulses();
    void systemAlarm();

    /* Getters */
    bool isAvailable();
    uint8_t getState();
    String getPixelAddressing();
    String getPixels();
    uint16_t getNumberOfLeds();
    uint8_t getDriver();
    bool getPower();
    uint8_t getMode();
    uint8_t getPowerAnimation();
    uint8_t getBrightness();

    /* Setters */
    void setBrightness(uint8_t brightness);

    /* Utility functions */
    CRGBW CRGBtoCRGBW(CRGB color);
    
  private:
    void _loadPixelAddresses();
    void _handleDoorOpen();
    void _handleDoorClosed();
    
    void _rotateLeft(uint8_t steps = 1);
    void _rotateRight(uint8_t steps = 1);
    CRGB _randomColor(uint8_t saturationPerc = 100);
    CRGB _blendColors(CRGB color1, float color1Portion, CRGB color2);
    CRGB _colorWheel(uint8_t position);
    CRGB _getHeatColor(uint8_t temperature, uint8_t pallete);
    uint8_t _getGradientColorPosition(uint8_t step);

    /* Modes (threads) */
    void __fade();
    void __gradient();
    void __blink();
    void __scan();
    void __theater();
    void __sine();
    void __bouncingBalls();
    void __dissolve();
    void __sparkle();
    void __fireworks();
    void __fire();
    void __sweep();
    void __colorTwinkels();
    void __meteorRain();
    void __colorWaves();
    void __modeTemplate1();
    void __modeTemplate2();
    void __modeTemplate3();
    void __modeTemplate4();
    void __modeTemplate5();
    void __modeTemplate6();
    void __modeTemplate7();
    void __modeTemplate8();
    void __modeTemplate9();
    void __modeTemplate10();

    
    void __systemPulses();
    void __systemAlarm();

    /* Power (on/off) animations */
    void _powerFade();
    void _powerDissolve();
    void _powerSweep();
    void _powerDualSweep();
    void _powerMultiSweep();

    /* Power (on/off) animations (threads) */
    void __powerFade();
    void __powerDissolve();
    void __powerSweep();
    void __powerDualSweep();
    void __powerMultiSweep();

    /* System functions */
    static void __startModeTask(void* parameter);
    void _waitUntilIdle();
    void _saveLeds();

    void _fadeBrightness();
    void _fadeToColor();
    void _fadeToMultipleColors(uint8_t desiredColorPos, bool fadeToGradientColors = false);
    
    void __fadeBrightness();
    void __fadeToColor();
    void __fadeToMultipleColors();

    /* Show functions */
    void _showLeds();

    /* Strip state */
    uint16_t _ledAddresses[MAX_NUMBER_LEDS];
    CRGB _leds[MAX_NUMBER_LEDS];
    CRGB _savedLeds[MAX_NUMBER_LEDS];

    CRGB _tempLeds[MAX_NUMBER_LEDS];
    CRGBW _crgbwTempLeds[MAX_NUMBER_LEDS];
    CRGB *_ledsPointer = (CRGB *) &_crgbwTempLeds[0];                           //Make pointer to array
    
    /* Pins */
    uint8_t _dataPin;
    uint8_t _clockPin;
    uint8_t _driver;
    uint16_t _numberLeds;
    uint16_t _highestPixelAddress;
    
    /* States */
    bool _isOn;
    bool _wasOn;
    uint8_t _brightness;
    uint8_t _prevBrightness;
    uint8_t _mode;
    uint8_t _prevMode;
    CRGB _fullColor;
    CRGB _prevColor;
    uint8_t _powerAnimation;
    bool _doorState;

    uint8_t _state;

    /* Temporary system variables*/
    uint8_t _desiredColorPos;
    bool _fadeToGradientColors;

    /* Mode parameters */ 
    ModeParameters _modeParameters[NUM_MODES]; 
    
    TaskHandle_t _taskHandler = NULL;                                           //One taskhandler, one task at a time

    Logger _l;

    MemoryManager _memoryManager;
    Preferences _nvMemory;
};
#endif