/******************************************************************************/
/*
 * File:    Ledstrip.cpp
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
#include "Ledstrip.h"

/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
Ledstrip::Ledstrip() {
    _l.setTag("Ledstrip");

    /* States */
    _state = _READY_TO_RUN;
    _prevBrightness = MAX_BRIGHTNESS;
    _isOn = true;
    _wasOn = true;
    _fullColor = CRGB(255, 255, 255);
    _doorState = false;

    /* Temporary variables */
    _desiredColorPos = 0;
    _fadeToGradientColors = false;
}

/******************************************************************************/
/*!
  @brief    Initializes the strip and starts the last known mode.
*/
/******************************************************************************/
void Ledstrip::initialize() {
    _l.logi("Initializing ledstrip", false);
    
    _nvMemory.begin(NV_MEM_CONFIG);
    _driver = _nvMemory.getUChar("driver", _SK6812);
    _numberLeds = _nvMemory.getUShort("numberLeds", MAX_NUMBER_LEDS);
    _powerAnimation = _nvMemory.getUChar("pwrAnimation", _POWER_FADE);
    _brightness = _nvMemory.getUChar("brightness", MAX_BRIGHTNESS);
    _mode = _nvMemory.getUChar("mode", MODE_COLOR);
    _nvMemory.end();
    
    _loadPixelAddresses();

    _l.logi("_driver: " + String(_driver));
    _l.logi("_numberLeds: " + String(_numberLeds));
    _l.logi("_powerAnimation: " + String(_powerAnimation));

    if (_driver == _WS2801) {
        FastLED.addLeds<WS2801, LEDSTRIP_DATA_PIN, LEDSTRIP_CLOCK_PIN, RBG>(_tempLeds, _numberLeds);
    } else if (_driver == _WS2812B) {
        FastLED.addLeds<WS2801, LEDSTRIP_DATA_PIN, RBG>(_tempLeds, _numberLeds);
    } else if (_driver == _SK6812) {
        FastLED.addLeds<WS2812B, LEDSTRIP_DATA_PIN, RGB>(_ledsPointer, getRGBWsize(_numberLeds));
    }
    
    for (uint8_t mode = 1; mode < NUM_MODES; mode++) {
        configureMode(mode, _memoryManager.loadModeParameters(mode), false);
    }

    setBrightness(_brightness);
    setMode(_mode);
}

/******************************************************************************/
/*!
  @brief    Sets the power.
  @param    state               Power state, true = on, false = off
  @param    startMode           If true, the mode is started
*/
/******************************************************************************/
void Ledstrip::setPower(bool state, bool startMode) {
    if (state == _isOn) {
        return;
    }
    
    _waitUntilIdle();
    
    if (_isOn) {
        _saveLeds();
    }
    
    _isOn = state;

    switch (_powerAnimation) {
        case _POWER_FADE:
            _powerFade();
            break;
        case _POWER_DISSOLVE:
            _powerDissolve();
            break;
        case _POWER_SWEEP:
            _powerSweep();
            break;
        case _POWER_DUAL_SWEEP:
            _powerDualSweep();
            break;
        default:
            break;
    }

    if (_isOn && startMode) {
        setMode(_mode);
    }
}

/******************************************************************************/
/*!
  @brief    Handles door changes.
  @param    state               State of door, true = open, false = close
*/
/******************************************************************************/
void Ledstrip::doorHandler(bool state) {
    if (state == _doorState) {
        return;
    }
    
    _waitUntilIdle();
    _doorState = state;
    
    if (state) {
        _handleDoorOpen();
    } else {
        _handleDoorClosed();
    }
}

/******************************************************************************/
/*!
  @brief    Handles the door opened event.
*/
/******************************************************************************/
void Ledstrip::_handleDoorOpen() {
    _wasOn = _isOn;
    //_isOn = false;
    _prevBrightness = _brightness;
    _prevColor = _fullColor;
    _prevMode = _mode;

    if (_isOn) {
        _modeParameters[MODE_COLOR].color1 = CRGB(255, 255, 255);
        color();
    } else {
        /* Fade to white leds */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _savedLeds[i] = CRGB(255, 255, 255);
        }
        
        setPower(true, false);
    }
    
    setBrightness(MAX_BRIGHTNESS);
    _waitUntilIdle();
    
    _state = _WAIT_FOR_DOOR_CLOSED;
}

/******************************************************************************/
/*!
  @brief    Handles the door closed event.
*/
/******************************************************************************/
void Ledstrip::_handleDoorClosed() {
    setBrightness(_prevBrightness);
    _waitUntilIdle();
    _modeParameters[MODE_COLOR].color1 = _prevColor;
    _mode = _prevMode;
    
    if (_wasOn) {
        setMode(_mode);
    } else {
        setPower(false, false);
        _waitUntilIdle();
        
        setMode(_mode);                                                         //Set mode to get leds values when turning on
        _waitUntilIdle();
        _saveLeds();
        
        /* Reset actual leds */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = CRGB(0, 0, 0);
        }
    }
}

/******************************************************************************/
/*!
  @brief    Starts the specified mode.
  @param    mode                Mode ID
*/
/******************************************************************************/
void Ledstrip::setMode(uint8_t mode) {
    switch (mode) {
        case MODE_COLOR:
            color();
            break;
        case MODE_FADE:
            fade();
            break;
        case MODE_GRADIENT:
            gradient();
            break;
        case MODE_BLINK:
            blink();
            break;
        case MODE_SCAN:
            scan();
            break;
        case MODE_THEATER:
            theater();
            break;
        case MODE_SINE:
            sine();
            break;
        case MODE_BOUNCING_BALLS:
            bouncingBalls();
            break;
        case MODE_DISSOLVE:
            dissolve();
            break;
        case MODE_SPARKLE:
            sparkle();
            break;
        case MODE_FIREWORKS:
            fireworks();
            break;
        case MODE_FIRE:
            fire();
            break;
        case MODE_SWEEP:
            sweep();
            break;
        case MODE_COLOR_TWINKELS:
            colorTwinkels();
            break;
        case MODE_METEOR_RAIN:
            meteorRain();
            break;
        case MODE_COLOR_WAVES:
            colorWaves();
            break;
        case MODE_TEMPLATE_1:
            modeTemplate1();
            break;
        case MODE_TEMPLATE_2:
            modeTemplate2();
            break;
        case MODE_TEMPLATE_3:
            modeTemplate3();
            break;
        case MODE_TEMPLATE_4:
            modeTemplate4();
            break;
        case MODE_TEMPLATE_5:
            modeTemplate5();
            break;
        case MODE_TEMPLATE_6:
            modeTemplate6();
            break;
        case MODE_TEMPLATE_7:
            modeTemplate7();
            break;
        case MODE_TEMPLATE_8:
            modeTemplate8();
            break;
        case MODE_TEMPLATE_9:
            modeTemplate9();
            break;
        case MODE_TEMPLATE_10:
            modeTemplate10();
            break;
        case MODE_DRAWING:
            _modeParameters[MODE_COLOR].color1 = CRGB(0, 0, 0);//Reset colors
            color();
            if (!_isOn) {
                setPower(true, false);
            }
            break;

        case SYSTEM_MODE_PULSES:
            systemPulses();
            break;
        case SYSTEM_MODE_ALARM:
            systemAlarm();
            break;
        default:
            _l.loge("Mode not found");
            break;
    }
    
    _mode = mode;
    _nvMemory.begin(NV_MEM_CONFIG);
    _nvMemory.putUChar("mode", mode);
    _nvMemory.end();
}

/******************************************************************************/
/*!
  @brief    Configures the specified mode.
  @param    mode                Mode ID
  @param    parameters          Parameters of the mode
  @param    save                If true, gets saved in non-volatile memory
*/
/******************************************************************************/
void Ledstrip::configureMode(uint8_t mode, ModeParameters parameters, bool save) {
    _modeParameters[mode] = parameters;

    if (save) {
        _memoryManager.writeModeParameters(mode, parameters);
    }

    _l.logd("Configured mode: " + String(mode));
}

/******************************************************************************/
/*!
  @brief    Sets the power animation.
  @param    animation           Animation to set
*/
/******************************************************************************/
void Ledstrip::setPowerAnimation(uint8_t animation) {
    _powerAnimation = animation;
    _nvMemory.begin(NV_MEM_CONFIG);
    _nvMemory.putUChar("pwrAnimation", animation);
    _nvMemory.end();
}

/******************************************************************************/
/*!
  @brief    Sets the pixel addressing configuration.
  @param    addressesJson       JSON string with addresses
  @param    numberOfLeds        Number of LEDs
*/
/******************************************************************************/
void Ledstrip::setPixelAddressing(String addressesJson, uint16_t numberOfLeds) {
    _nvMemory.begin(NV_MEM_CONFIG);
    _nvMemory.putString("ledAddresses", addressesJson);
    _nvMemory.putUShort("numberLeds", numberOfLeds);
    _nvMemory.end();
        
    JsonDocument jsonParser;

    deserializeJson(jsonParser, addressesJson);                                 //Convert JSON string to object
    
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        _ledAddresses[i] = (uint16_t) jsonParser[i];
    }
}

/******************************************************************************/
/*!
  @brief    Draws the specified LEDs.
  @param    leds                LEDs array
*/
/******************************************************************************/
void Ledstrip::drawPixels(CRGB leds[]) {
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        _leds[i] = leds[i];
    }
    
    _showLeds();
}

#pragma region Modes
/******************************************************************************/
/*!
  @brief    Fade leds to the specified color.
*/
/******************************************************************************/
void Ledstrip::color() {
    _fullColor = _modeParameters[MODE_COLOR].color1;
    _fadeToColor();

    _waitUntilIdle();

    _l.logi("Color mode");
    
    _mode = MODE_COLOR;
    _state = _READY_TO_RUN;
}

/******************************************************************************/
/*!
  @brief    Fade of all colors possible, all LEDs same color.
*/
/******************************************************************************/
void Ledstrip::fade() {
    _fullColor = _colorWheel(_modeParameters[MODE_FADE].colorPosition);
    
    _fadeToColor();

    _waitUntilIdle();
    
    _mode = MODE_FADE;
    _state = _LOOPING;
    
    _l.logi("Start fade mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority, 1 seems to work just fine for us
        &_taskHandler,                                                          //Task handle
        CORE_NUMBER                                                             //Task CPU core, 1 is where usually the Arduino Framework code (setup and loop function) are running,
                                                                                //core 0 by default runs the Wifi Stack
    );
}

/******************************************************************************/
/*!
  @brief    Task. Fade of all colors possible.
*/
/******************************************************************************/
void Ledstrip::__fade() {
    while (1) {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = _colorWheel(_modeParameters[MODE_FADE].colorPosition);
        }

        _showLeds();
        vTaskDelay(_modeParameters[MODE_FADE].delay);
        
        _modeParameters[MODE_FADE].colorPosition++;
    }
}

/******************************************************************************/
/*!
  @brief    Fade of gradient tints in a rainbow style across strip.
*/
/******************************************************************************/
void Ledstrip::gradient() {
    _fadeToMultipleColors(_modeParameters[MODE_GRADIENT].colorPosition, true);

    _waitUntilIdle();
    
    _mode = MODE_GRADIENT;
    _state = _LOOPING;

    _l.logi("Start gradient mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Fade of gradient tints in a rainbow style across strip.
           
*/
/******************************************************************************/
void Ledstrip::__gradient() {
    int8_t direction = 1;

    while (1) {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            uint8_t colorPosition = _getGradientColorPosition(i);
            _leds[i] = _colorWheel(colorPosition);
            _leds[_highestPixelAddress-1 - i] = _colorWheel(colorPosition);     //gradient begins on right and left side and ends in middle, so split strip in half
        }
        
        _showLeds();
        vTaskDelay(_modeParameters[MODE_GRADIENT].delay);
        _modeParameters[MODE_GRADIENT].colorPosition += direction;
        
        if (_modeParameters[MODE_GRADIENT].colorPosition > _modeParameters[MODE_GRADIENT].maxColorPos) {
            direction = -1;
        } else if (_modeParameters[MODE_GRADIENT].colorPosition < _modeParameters[MODE_GRADIENT].minColorPos) {
            direction = 1;
        }
    }
}

/******************************************************************************/
/*!
  @brief    Blinking between two colors.
*/
/******************************************************************************/
void Ledstrip::blink() {
    if (_modeParameters[MODE_BLINK].useGradient1) {
        _fadeToMultipleColors(0);
    } else {
        _fullColor = _modeParameters[MODE_BLINK].color1;
        _fadeToColor();
    }
    
    _waitUntilIdle();
     
    _mode = MODE_BLINK;
    _state = _LOOPING;

    _l.logi("Start blink mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Blinking between two colors.
*/
/******************************************************************************/
void Ledstrip::__blink() {
    uint8_t colorPosition1 = 0;
    uint8_t colorPosition2 = 255;
    int8_t colorDirection1 = 1;
    int8_t colorDirection2 = -1;

    while (1) {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (_modeParameters[MODE_BLINK].useGradient1) {
                _leds[i] = _colorWheel((i + colorPosition1) & 255);
            } else {
                _leds[i] = _modeParameters[MODE_BLINK].color1;
            }
        }

        _showLeds();
        vTaskDelay(_modeParameters[MODE_BLINK].delay);

        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (_modeParameters[MODE_BLINK].useGradient2) {
                _leds[i] = _colorWheel((i + colorPosition2) & 255);
            } else {
                _leds[i] = _modeParameters[MODE_BLINK].color2;
            }
        }

        _showLeds();
        vTaskDelay(_modeParameters[MODE_BLINK].delay);
        
        if (_modeParameters[MODE_BLINK].useGradient1) {
            colorPosition1 += colorDirection1;
            if (colorPosition1 == 255 || colorPosition1 == 0) {
                colorDirection1 = -colorDirection1;
            }
        }
        if (_modeParameters[MODE_BLINK].useGradient2) {
            colorPosition2 += colorDirection2;
            if (colorPosition2 == 255 || colorPosition2 == 0) {
                colorDirection2 = -colorDirection2;
            }
        }
    }
}

/******************************************************************************/
/*!
  @brief    Moving dot/segment between endpoints.
*/
/******************************************************************************/
void Ledstrip::scan() {
    if (_modeParameters[MODE_SCAN].useGradient2) {
        _fadeToMultipleColors(0);
    } else {
        _fullColor = _modeParameters[MODE_SCAN].color2;
        _fadeToColor();
    }

    _waitUntilIdle();
     
    _mode = MODE_SCAN;
    _state = _LOOPING;
    
    _l.logi("Start scan mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Moving dot/segment between endpoints.
*/
/******************************************************************************/
void Ledstrip::__scan() {
    uint16_t padding = _modeParameters[MODE_SCAN].segmentSize + _modeParameters[MODE_SCAN].tailLength;
    uint16_t segmentLocation = padding;                                         //Padding because there is where the leds will start to shine
    int8_t segmentDirection = 1;

    uint8_t colorPosition1 = 0;
    uint8_t colorPosition2 = 255;
    int8_t colorDirection1 = 1;
    int8_t colorDirection2 = -1;
    
    while (1) {
        /* Draw background */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (_modeParameters[MODE_SCAN].useGradient2) {
                _leds[i] = _colorWheel((i + colorPosition2) & 255);
            } else {
                _leds[i] = _modeParameters[MODE_SCAN].color2;
            }
        }

        /* Draw tail */
        float color1Portion = 0;
        CRGB tailColor;
        CRGB color1;
        CRGB color2;
        
        if (_modeParameters[MODE_SCAN].useGradient1) {
            color1 = _colorWheel(colorPosition1 & 255);
        } else {
            color1 = _modeParameters[MODE_SCAN].color1;
        }
        if (_modeParameters[MODE_SCAN].useGradient2) {
            color2 = _colorWheel(colorPosition2 & 255);
        } else {
            color2 = _modeParameters[MODE_SCAN].color2;
        }

        for (uint8_t i = 0; i < _modeParameters[MODE_SCAN].tailLength; i++) {
            color1Portion = (_modeParameters[MODE_SCAN].tailLength - i) * 1.0 / _modeParameters[MODE_SCAN].tailLength;
            color1Portion = color1Portion / 2;
            
            tailColor = _blendColors(color1, color1Portion, color2);
            
            if (segmentDirection == 1 && segmentLocation - padding - _modeParameters[MODE_SCAN].segmentSize - i >= 0) {
                _leds[segmentLocation - padding - _modeParameters[MODE_SCAN].segmentSize - i] = tailColor;
            } else if (segmentDirection == -1 && segmentLocation - padding + i < _highestPixelAddress) {
                _leds[segmentLocation - padding + i] = tailColor;
            }
        }
        
        /* Draw scan leds */
        for (uint8_t i = 0; i < _modeParameters[MODE_SCAN].segmentSize; i++) {
            if (segmentLocation - padding - i < 0) {
                break;
            }
            if (_modeParameters[MODE_SCAN].useGradient1) {
                _leds[segmentLocation - padding - i] = _colorWheel((i + colorPosition1) & 255);
            } else {
                _leds[segmentLocation - padding - i] = _modeParameters[MODE_SCAN].color1;
            }
        }
        
        _showLeds();
        if (_modeParameters[MODE_SCAN].useGradient1) {
            colorPosition1 += colorDirection1;
            if (colorPosition1 == 255 || colorPosition1 == 0) {
                colorDirection1 = -colorDirection1;
            }
        }
        if (_modeParameters[MODE_SCAN].useGradient2) {
            colorPosition2 += colorDirection2;
            if (colorPosition2 == 255 || colorPosition2 == 0) {
                colorDirection2 = -colorDirection2;
            }
        }

        /* No need to wait if no scanline has been drawn */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {//TODO can be deletd?
            /* If something is drawn, wait */
            if (_leds[i] != _modeParameters[MODE_SCAN].color2) {
                vTaskDelay(_modeParameters[MODE_SCAN].delay);
                break;
            }
        }

        /* Change directions */
        if (segmentLocation >= _highestPixelAddress + padding*2) {
            segmentDirection = -1;
        } else if (segmentLocation == 0) {
            segmentDirection = 1;
        }
        segmentLocation += segmentDirection;
    }
}

/******************************************************************************/
/*!
  @brief    Pattern used in old theaters.
*/
/******************************************************************************/
void Ledstrip::theater() {
    if (_modeParameters[MODE_THEATER].useGradient2) {
        _fadeToMultipleColors(0);
    } else {
        _fullColor = _modeParameters[MODE_THEATER].color2;
        _fadeToColor();
    }
    
    _fadeToColor();

    _waitUntilIdle();
     
    _mode = MODE_THEATER;
    _state = _LOOPING;
    
    _l.logi("Start theater mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Pattern used in old theaters. NOT FINISHED TODO
*/
/******************************************************************************/
void Ledstrip::__theater() {
    uint8_t colorToggle = 0;
    uint8_t dotCounter = 0;

    uint8_t colorPosition1 = 0;
    uint8_t colorPosition2 = 255;
    
    /* Draw segments */
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        if (dotCounter == _modeParameters[MODE_THEATER].segmentSize) {
            dotCounter = 0;
            colorToggle = !colorToggle;
        }
        
        if (colorToggle) {
            if (_modeParameters[MODE_THEATER].useGradient1) {
                _leds[i] = _colorWheel(colorPosition1);
                colorPosition1++;
            } else {
                _leds[i] = _modeParameters[MODE_THEATER].color1;
            }
        } else {
            if (_modeParameters[MODE_THEATER].useGradient2) {
                _leds[i]  = _colorWheel(colorPosition2);
                colorPosition2--;
            } else {
                _leds[i] = _modeParameters[MODE_THEATER].color2;
            }
        }
        dotCounter++;
    }

    while (1) {
        if (_modeParameters[MODE_THEATER].direction == DIRECTION_LEFT) {
            _rotateLeft();
        } else {
            _rotateRight();
        }

        _showLeds();
        vTaskDelay(_modeParameters[MODE_THEATER].delay);
    }
}

/******************************************************************************/
/*!
  @brief    Sine waves scrolling.
*/
/******************************************************************************/
void Ledstrip::sine() {
    _fullColor = _modeParameters[MODE_SINE].color2;
    
    _fadeToColor();

    _waitUntilIdle();
     
    _mode = MODE_SINE;
    _state = _LOOPING;
    
    _l.logi("Start sine mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Sine waves scrolling. NOT FINISHED TODO
*/
/******************************************************************************/
void Ledstrip::__sine() {
    float speed = 0.1;
    float time = 0;
    float colorPortion;
    uint16_t ledAddress;
    //_modeParameters[MODE_SINE].waveLength MIN 1 MAX 20 todo
    while (1) {
        if (_modeParameters[MODE_SINE].direction == DIRECTION_LEFT) {
            time += speed;
        } else {
            time -= speed;
        }

        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            colorPortion = (sin(i*2.0/_modeParameters[MODE_SINE].waveLength + time) + 1) / 2;//*2 because otherwise wavelength is too big

            if (_modeParameters[MODE_SINE].useGradient1) {
                _leds[i] = CHSV((_modeParameters[MODE_SINE].colorPosition + i * 5) % 255, 255, colorPortion * 255);
            } else {
                _leds[i] = _blendColors(_modeParameters[MODE_SINE].color2, colorPortion, _modeParameters[MODE_SINE].color1);
            }
        }
        _modeParameters[MODE_SINE].colorPosition++;

        _showLeds();
        vTaskDelay(_modeParameters[MODE_SINE].delay);
    }
}

/******************************************************************************/
/*!
  @brief    Bouncing balls simulation.
*/
/******************************************************************************/
void Ledstrip::bouncingBalls() {
    _fullColor = _modeParameters[MODE_BOUNCING_BALLS].color2;
    
    _fadeToColor();

    _waitUntilIdle();
     
    _mode = MODE_BOUNCING_BALLS;
    _state = _LOOPING;
    
    _l.logi("Start bouncingBalls mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Bouncing balls simulation.
*/
/******************************************************************************/
void Ledstrip::__bouncingBalls() {
    uint8_t numberOfBalls = _modeParameters[MODE_BOUNCING_BALLS].numberOfElements;
    uint8_t ballSize = _modeParameters[MODE_BOUNCING_BALLS].segmentSize;
    const float GRAVITY = -9.81;
    const uint8_t START_HEIGHT = 10;
    const float IMPACT_VELOCITY_START = sqrt(-2 * GRAVITY * START_HEIGHT);
   
    float height[numberOfBalls];
    float impactVelocity[numberOfBalls];
    float timeSinceLastBounce[numberOfBalls];
    uint16_t position[numberOfBalls];
    uint64_t clockTimeSinceLastBounce[numberOfBalls];
    float dampening[numberOfBalls];
    CRGB ballColors[numberOfBalls];
    
    for (uint8_t i = 0; i < numberOfBalls; i++) {  
        clockTimeSinceLastBounce[i] = millis();
        height[i] = START_HEIGHT;
        position[i] = 0;
        impactVelocity[i] = IMPACT_VELOCITY_START;
        timeSinceLastBounce[i] = 0;
        dampening[i] = 0.90 - float(i)/pow(numberOfBalls, 2);

        if (_modeParameters[MODE_BOUNCING_BALLS].useGradient1) {
            ballColors[i] = _colorWheel(random8());
        } else {
            ballColors[i] = _modeParameters[MODE_BOUNCING_BALLS].color1;
        }
    }

    while (1) {
        /* Reset leds */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = _modeParameters[MODE_BOUNCING_BALLS].color2;
        }

        for (uint8_t i = 0; i < numberOfBalls; i++) {
            timeSinceLastBounce[i] =  millis() - clockTimeSinceLastBounce[i];
            height[i] = 0.5 * GRAVITY * pow(timeSinceLastBounce[i]/1000 , 2.0) + impactVelocity[i] * timeSinceLastBounce[i]/1000;
        
            if (height[i] < 0) {                      
                height[i] = 0;
                impactVelocity[i] = dampening[i] * impactVelocity[i];
                clockTimeSinceLastBounce[i] = millis();
        
                if (impactVelocity[i] < 0.01) {
                    impactVelocity[i] = IMPACT_VELOCITY_START;
                }
            }
            position[i] = round( height[i] * (_highestPixelAddress - 1) / START_HEIGHT);
        }
     
        /* Draw balls */
        for (uint8_t i = 0 ; i < numberOfBalls ; i++) {
            for (uint8_t ballPixel = 0; ballPixel < ballSize; ballPixel++) {
                _leds[position[i] + ballPixel] = ballColors[i];
            }
        }
       
        _showLeds();
        vTaskDelay(10);
    }
}

/******************************************************************************/
/*!
  @brief    Two colors dissolving in each other.
*/
/******************************************************************************/
void Ledstrip::dissolve() {
    _fullColor = _modeParameters[MODE_DISSOLVE].color2;
    
    _fadeToColor();

    _waitUntilIdle();
     
    _mode = MODE_DISSOLVE;
    _state = _LOOPING;
    
    _l.logi("Start dissolve mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Two colors dissolving in each other.
*/
/******************************************************************************/
void Ledstrip::__dissolve() {
    bool colorToggle = false;
    uint16_t randomIndex = 0;
    uint16_t temp = 0;
    uint16_t indexes[_highestPixelAddress];
    
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        indexes[i] = i;
    }
        
    while (1) {
        /* Shuffle order on randomized index */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            randomIndex = random(i, _highestPixelAddress);                      //Generate a random index between i and n, index < i is already randomized
            
            /* Swap elements */
            temp = indexes[i];
            indexes[i] = indexes[randomIndex];
            indexes[randomIndex] = temp;
        }

        /* Draw randomized leds */
        float color1Portion = 0;
        CRGB dotColor;
        
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (_modeParameters[MODE_DISSOLVE].timeFade == 0) {
                if (colorToggle) {
                    _leds[indexes[i]] = _modeParameters[MODE_DISSOLVE].color1;
                } else {                  
                    _leds[indexes[i]] = _modeParameters[MODE_DISSOLVE].color2;
                }
                _showLeds();
                vTaskDelay(_modeParameters[MODE_DISSOLVE].delay);
                continue;
            }

            for (uint16_t timeStep = 0; timeStep < 100; timeStep++) {
                if (colorToggle) {
                    color1Portion = (100-timeStep-1) * 1.0 / 100;
                } else {                  
                    color1Portion = timeStep * 1.0 / 100;
                }
                
                color1Portion = color1Portion / 2;
  
                dotColor = _blendColors(_modeParameters[MODE_DISSOLVE].color1, color1Portion, _modeParameters[MODE_DISSOLVE].color2);
  
                _leds[indexes[i]] = dotColor;
  
                _showLeds();
                vTaskDelay(_modeParameters[MODE_DISSOLVE].timeFade/100);
            }
            vTaskDelay(_modeParameters[MODE_DISSOLVE].delay);
        }

        vTaskDelay(_modeParameters[MODE_DISSOLVE].delayBetween);
        colorToggle = !colorToggle;
    }
}

/******************************************************************************/
/*!
  @brief    One LED at a time sparkles and fades away.
*/
/******************************************************************************/
void Ledstrip::sparkle() {
    _fullColor = _modeParameters[MODE_SPARKLE].color2;
    
    _fadeToColor();

    _waitUntilIdle();
     
    _mode = MODE_SPARKLE;
    _state = _LOOPING;
    
    _l.logi("Start sparkle mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. One LED at a time sparkles and fades away.
*/
/******************************************************************************/
void Ledstrip::__sparkle() {
    uint8_t colorToggle = 0;
    uint16_t randomIndex = 0;
    uint16_t temp = 0;
    uint16_t indexes[_highestPixelAddress];
    
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        indexes[i] = i;
    }
    
    while (1) {    
        /* Shuffle order on randomized index */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            randomIndex = random(i, _highestPixelAddress);                      //Generate a random index between i and n, index < i is already randomized
            
            /* Swap elements */
            temp = indexes[i];
            indexes[i] = indexes[randomIndex];
            indexes[randomIndex] = temp;
        }

        /* Draw randomized leds */
        float color1Portion = 0;
        CRGB dotColor;
        
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[indexes[i]] = _modeParameters[MODE_SPARKLE].color1;

            if (_modeParameters[MODE_SPARKLE].timeFade == 0) {
                _showLeds();
                vTaskDelay(_modeParameters[MODE_SPARKLE].delayBetween);
                _leds[indexes[i]] = _modeParameters[MODE_SPARKLE].color2;
                continue;
            }

            for (uint16_t timeStep = 0; timeStep < 100; timeStep++) {
                color1Portion = (100-timeStep-1) * 1.0 / 100;
                color1Portion = color1Portion / 2;
  
                dotColor = _blendColors(_modeParameters[MODE_SPARKLE].color1, color1Portion, _modeParameters[MODE_SPARKLE].color2);
  
                _leds[indexes[i]] = dotColor;
  
                _showLeds();
                vTaskDelay(_modeParameters[MODE_SPARKLE].timeFade/100);
            }
            vTaskDelay(_modeParameters[MODE_SPARKLE].delayBetween);
        }
    }
}

/******************************************************************************/
/*!
  @brief    Fireworks simulation.
*/
/******************************************************************************/
void Ledstrip::fireworks() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = MODE_FIREWORKS;
    _state = _LOOPING;
    
    _l.logi("Start fireworks mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Random color blobs light up, then fade away. TODO IMPLEMENT
*/
/******************************************************************************/
void Ledstrip::__fireworks() {
    while (1) {
        vTaskDelay(1000);
        _showLeds();
    }
}

/******************************************************************************/
/*!
  @brief    Fire simulation.
*/
/******************************************************************************/
void Ledstrip::fire() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = MODE_FIRE;
    _state = _LOOPING;
    
    _l.logi("Start fire mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Fire simulation.
*/
/******************************************************************************/
void Ledstrip::__fire() {
    const uint8_t COOLING = 120;
    const uint8_t SPARKING = 100;

    uint8_t heat[_highestPixelAddress];
    int cooldown;

    while (1) {
        /* Cool down every cell a little */
        for(uint16_t i = 0; i < _highestPixelAddress; i++) {
            cooldown = random(0, (((COOLING-_modeParameters[MODE_FIRE].segmentSize) * 10) / _highestPixelAddress) + 2);
            
            if (cooldown > heat[i]) {
                heat[i] = 0;
            } else {
                heat[i] = heat[i] - cooldown;
            }
        }
       
        /* Heat from each cell drifts 'up' and diffuses a little */
        for(uint16_t k = _highestPixelAddress - 1; k >= 2; k--) {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }
         
        /* Randomly ignite new 'sparks' near the bottom */
        if(random(255) < SPARKING) {
            uint16_t y = random(_highestPixelAddress/10);
            heat[y] = heat[y] + random(160,255);
        }
      
        /* Convert heat to LED colors */
        for(uint16_t j = 0; j < _highestPixelAddress; j++) {
            _leds[j] = _getHeatColor(heat[j], _modeParameters[MODE_FIRE].palette);
        }
      
        _showLeds();
        vTaskDelay(20);//_modeParameters[MODE_FIRE].delay); todo
    }
}

/******************************************************************************/
/*!
  @brief    A sweep between two colors.
*/
/******************************************************************************/
void Ledstrip::sweep() {
    if (_modeParameters[MODE_SWEEP].useGradient1) {
        _fadeToMultipleColors(0);
    } else {
        _fullColor = _modeParameters[MODE_SWEEP].color1;
        _fadeToColor();        
    }
    _waitUntilIdle();
     
    _mode = MODE_SWEEP;
    _state = _LOOPING;
    
    _l.logi("Start sweep mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. A sweep between two colors.
*/
/******************************************************************************/
void Ledstrip::__sweep() {
    float color1Portion = 0;
    uint16_t animationLength = _highestPixelAddress + _modeParameters[MODE_SWEEP].fadeLength;
    CRGB color1 = _modeParameters[MODE_SWEEP].color1;
    CRGB color2 = _modeParameters[MODE_SWEEP].color2;
    CRGB leds1[animationLength];
    CRGB leds2[animationLength];
    uint8_t colorPosition1 = 0;
    uint8_t colorPosition2 = 255;
    int8_t colorDirection1 = 1;
    int8_t colorDirection2 = -1;
    bool color1Main = false;

    while (1) {
        if (_modeParameters[MODE_SWEEP].useGradient1) {
            for (uint16_t i = 0; i < animationLength; i++) {
                leds1[i] = _colorWheel((i*3 + colorPosition1) & 255);
            }
        } else {
            for (uint16_t i = 0; i < animationLength; i++) {
                leds1[i] = _modeParameters[MODE_SWEEP].color1;
            }
        }
        if (_modeParameters[MODE_SWEEP].useGradient2) {
            for (uint16_t i = 0; i < animationLength; i++) {
                leds2[i] = _colorWheel((i*3 + colorPosition2) & 255);
            }
        } else {
            for (uint16_t i = 0; i < animationLength; i++) {
                leds2[i] = _modeParameters[MODE_SWEEP].color2;
            }
        }

        /* Animate */
        for (uint16_t i = 0; i < animationLength; i++) {
            for (uint8_t j = 0; j < _modeParameters[MODE_SWEEP].fadeLength; j++) {
                color1Portion = (_modeParameters[MODE_SWEEP].fadeLength-j) * 1.0 / _modeParameters[MODE_SWEEP].fadeLength;
                
                int16_t ledIndex = i + j - _modeParameters[MODE_SWEEP].fadeLength;
                if (ledIndex < 0 || ledIndex > _highestPixelAddress) {
                    continue;
                }

                if (color1Main) {
                    _leds[ledIndex] = _blendColors(leds1[i + j], color1Portion, leds2[i + j]);
                } else {
                    _leds[ledIndex] = _blendColors(leds2[i + j], color1Portion, leds1[i + j]);
                }
            }
            _showLeds();
            vTaskDelay(_modeParameters[MODE_SWEEP].delay);
        }

        if (_modeParameters[MODE_SWEEP].useGradient1) {
            colorPosition1 += colorDirection1;
            if (colorPosition1 == 255 || colorPosition1 == 0) {
                colorDirection1 = -colorDirection1;
            }
        }
        if (_modeParameters[MODE_SWEEP].useGradient2) {
            colorPosition2 += colorDirection2;
            if (colorPosition2 == 255 || colorPosition2 == 0) {
                colorDirection2 = -colorDirection2;
            }
        }

        color1Main = !color1Main;
        vTaskDelay(_modeParameters[MODE_SWEEP].delayBetween);
    }
}

/******************************************************************************/
/*!
  @brief    Random color blobs light up, then fade away.
*/
/******************************************************************************/
void Ledstrip::colorTwinkels() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = MODE_COLOR_TWINKELS;
    _state = _LOOPING;
    
    _l.logi("Start color twinkel mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Random color blobs light up, then fade away. TODO test
*/
/******************************************************************************/
void Ledstrip::__colorTwinkels() {
    uint8_t fadeIntensity = MAX_FADE_TIME+1 - _modeParameters[MODE_COLOR_TWINKELS].timeFade;    //Lower = slower fade rate.
    int16_t hue = 50;                                                           //Starting hue.
    uint16_t hueRange = 256;                                                    //Range of random #'s to use for hue
    uint8_t secondHand;
    uint8_t lastSecond = 99;                                                    // Static variable, means it's only defined once. This is our 'debounce' variable.

    CRGBPalette16 currentPalette = CloudColors_p;
    CRGBPalette16 targetPalette = CloudColors_p;
    _modeParameters[MODE_COLOR_TWINKELS].palette = PALETTE_RANDOM;
    if (_modeParameters[MODE_COLOR_TWINKELS].palette != PALETTE_RANDOM) {
        switch (_modeParameters[MODE_COLOR_TWINKELS].palette) {
            case PALETTE_CLOUD_COLORS:
                currentPalette = CloudColors_p;
                break;
            case PALETTE_LAVA_COLORS:
                currentPalette = LavaColors_p;
                break;
            case PALETTE_OCEAN_COLORS:
                currentPalette = OceanColors_p;
                break;
            case PALETTE_FOREST_COLORS:
                currentPalette = ForestColors_p;
                break;
        
        default:
            break;
        }
    }

    while (1) {
        secondHand = (millis() % (_modeParameters[MODE_COLOR_TWINKELS].delayBetween * 4) / 1000);
        
        if (_modeParameters[MODE_COLOR_TWINKELS].palette == PALETTE_RANDOM) {
            if (lastSecond != secondHand) {                                     //Debounce to make sure we're not repeating an assignment.
                lastSecond = secondHand;
                _l.logd(String(lastSecond));
                if (secondHand == _modeParameters[MODE_COLOR_TWINKELS].delayBetween) {
                    targetPalette = CloudColors_p;
                    hue = 192;
                    hueRange = 256;
                } else if (secondHand == _modeParameters[MODE_COLOR_TWINKELS].delayBetween * 2) {
                    targetPalette = LavaColors_p;
                    hue = 128;
                    hueRange = 64;
                } else if (secondHand == _modeParameters[MODE_COLOR_TWINKELS].delayBetween * 3) {
                    targetPalette = OceanColors_p;
                    hue = 128;
                    hueRange = 64;
                } else if (secondHand == _modeParameters[MODE_COLOR_TWINKELS].delayBetween * 4) {
                    targetPalette = ForestColors_p;
                    hue = random16(255);
                    hueRange = 16;
                }
            }

            EVERY_N_MILLISECONDS(100) {
                nblendPaletteTowardPalette(currentPalette, targetPalette);
            }
        }

        fadeToBlackBy(_leds, _highestPixelAddress, fadeIntensity);
        uint16_t position = random16(_highestPixelAddress);                     //Pick an LED at random.
        _leds[position] = ColorFromPalette(currentPalette, hue + random16(hueRange)/4);
        hue++;

        _showLeds();
        vTaskDelay(1);//_modeParameters[MODE_COLOR_TWINKELS].delay);
    }
}

/******************************************************************************/
/*!
  @brief    Meteor rain simulation.
*/
/******************************************************************************/
void Ledstrip::meteorRain() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = MODE_METEOR_RAIN;
    _state = _LOOPING;
    
    _l.logi("Start color meteor mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Meteor rain simulation.
*/
/******************************************************************************/
void Ledstrip::__meteorRain() {
    uint8_t meteorSize = _modeParameters[MODE_METEOR_RAIN].segmentSize;
    uint8_t meteorTrailDecay = _modeParameters[MODE_METEOR_RAIN].tailLength;

    while (1) {
        /* Reset leds */
        //for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        //    _leds[i] = CRGB(0, 0, 0);
        //}
 
        for (uint16_t i = 0; i < _highestPixelAddress + meteorTrailDecay; i++) {
            // fade brightness all LEDs one step
            for(int j = 0; j < _highestPixelAddress; j++) {
                if (random(10) > 5) {
                    _leds[j].fadeToBlackBy(meteorTrailDecay);
                }
            }
            
            // draw meteor
            for (int j = 0; j < meteorSize; j++) {
                if ((i - j < _highestPixelAddress) && (i - j >= 0)) {
                    _leds[i-j] = _modeParameters[MODE_METEOR_RAIN].color1;
                }
            }
            
            _showLeds();
            vTaskDelay(_modeParameters[MODE_METEOR_RAIN].delay);
        }
        //vTaskDelay(_modeParameters[MODE_METEOR_RAIN].delayBetween);
        //vTaskDelay(_modeParameters[MODE_METEOR_RAIN].randomnessDelay);todo implement randomness delauy
    }
}

/******************************************************************************/
/*!
  @brief    Waves of different colors.
*/
/******************************************************************************/
void Ledstrip::colorWaves() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = MODE_COLOR_WAVES;
    _state = _LOOPING;
    
    _l.logi("Start color waves mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. Waves of different colors.
*/
/******************************************************************************/
void Ledstrip::__colorWaves() {
    CRGBPalette16 currentPalette = RainbowColors_p;
    CRGBPalette16 targetPalette;
    //TODO implement palette

    while (1) {
        uint8_t wave1 = beatsin8(4, 0, 255);                                    //That's the same as beatsin8(9);
        uint8_t wave2 = beatsin8(3, 0, 255);
        uint8_t wave3 = beatsin8(2, 0, 255);
        uint8_t wave4 = beatsin8(1, 0, 255);

        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = ColorFromPalette(currentPalette, i+wave1+wave2+wave3+wave4); 
        }

        EVERY_N_MILLISECONDS(100) {
            nblendPaletteTowardPalette(currentPalette, targetPalette);          //Palette blending capability.
        }
        
        EVERY_N_SECONDS(5) {                                                    //Change the target palette to a random one every 5 seconds.
            targetPalette = CRGBPalette16(
                                            CHSV(random8(), 255, random8(128,255)),
                                            CHSV(random8(), 255, random8(128,255)),
                                            CHSV(random8(), 192, random8(128,255)),
                                            CHSV(random8(), 255, random8(128,255))
                                        );
        }
        
        _showLeds();
        vTaskDelay(1);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate1() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = MODE_TEMPLATE_1;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_1 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate1() {

    while (1) {

        _showLeds();
        vTaskDelay(1);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate2() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_2;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_2 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate2() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_2].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate3() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                           //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_3;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_3 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate3() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_3].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate4() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_4;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_4 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate4() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_4].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate5() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_5;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_5 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate5() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_5].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate6() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_6;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_6 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate6() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_6].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate7() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_7;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_7 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate7() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_7].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate8() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_8;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_8 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate8() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_8].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate9() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                             //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_9;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_9 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate9() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_9].delay);
    }
}

/******************************************************************************/
/*!
  @brief    MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::modeTemplate10() {
    //_fullColor = CRGB(0, 0, 0);
    //_fadeToColor();                                                           //Black background

    //_waitUntilIdle();
     
    _mode = MODE_TEMPLATE_10;
    _state = _LOOPING;
    
    _l.logi("Start MODE_TEMPLATE_10 mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Task. MODE TEMPLATE TO BE IMPLEMENTED.
*/
/******************************************************************************/
void Ledstrip::__modeTemplate10() {
    
    while (1) {
        _showLeds();
        vTaskDelay(_modeParameters[MODE_TEMPLATE_10].delay);
    }
}
#pragma endregion


#pragma region System modes
/******************************************************************************/
/*!
  @brief    Random color blobs light up, then fade away.
  @param    color1          Color 1
  @param    color2          Color 2
  @param    delayBetween    Delay between fireworks (in ms)
*/
/******************************************************************************/
void Ledstrip::systemPulses() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = SYSTEM_MODE_PULSES;
    _state = _LOOPING;
    
    _l.logi("Start pulses mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Random color blobs light up, then fade away.
*/
/******************************************************************************/
void Ledstrip::__systemPulses() {
    uint16_t padding = 20;
    uint16_t segmentLocation = padding;                                         //Padding because there is where the leds will start to shine
    int8_t segmentDirection = 1;
    
    while (1) {
        /* Draw background */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = CRGB(0,0,0);
        }

        /* Draw tail */
        float color1Portion = 0;
        CRGB tailColor;
        CRGB color1 = CRGB (255,255,255);
        CRGB color2 = CRGB (0,0,0);
        
        for (uint8_t i = 0; i < padding; i++) {
            color1Portion = (padding - i) * 1.0 / padding;
            color1Portion = color1Portion / 2;
            
            tailColor = _blendColors(color1, color1Portion, color2);
            
            if (segmentLocation - padding - i >= 0) {
                _leds[segmentLocation - padding - i] = tailColor;
            }
            if (segmentLocation - padding + i < _highestPixelAddress) {
                _leds[segmentLocation - padding + i] = tailColor;
            }
        }
        
        _showLeds();

        /* No need to wait if no scanline has been drawn */
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            /* If something is drawn, wait */
            if (_leds[i] != color2) {
                vTaskDelay(50);
                break;
            }
        }

        /* Change directions */
        if (segmentLocation >= _highestPixelAddress + padding*2) {
            segmentDirection = -1;
        } else if (segmentLocation == 0) {
            segmentDirection = 1;
        }
        segmentLocation += segmentDirection;
    }
}

/******************************************************************************/
/*!
  @brief    Random color blobs light up, then fade away.
  @param    color1          Color 1
  @param    color2          Color 2
  @param    delayBetween    X
*/
/******************************************************************************/
void Ledstrip::systemAlarm() {
    _fullColor = CRGB(0, 0, 0);
    _fadeToColor();                                                             //Black background

    _waitUntilIdle();
     
    _mode = SYSTEM_MODE_ALARM;
    _state = _LOOPING;
    
    _l.logi("Start alarm mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "ModeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Random color blobs light up, then fade away.
*/
/******************************************************************************/
void Ledstrip::__systemAlarm() {
    uint8_t cycle = 0;

    while (1) {
        /* Flash and then wait */
        for (uint8_t flash = 0; flash < 4; flash++) {
            for (uint16_t i = 0; i < _highestPixelAddress; i++) {
                _leds[i] = CRGB(255, 255, 255);
            }
            _showLeds();
            vTaskDelay(25);
            for (uint16_t i = 0; i < _highestPixelAddress; i++) {
                _leds[i] = CRGB(0, 0, 0);
            }
            _showLeds();
            vTaskDelay(150);
        }

        /* If alarm is on for long, flash continuously */
        if (cycle < 50) {
            vTaskDelay(750);
            cycle++;
        }
    }
}
#pragma endregion

#pragma region Power functionality
/******************************************************************************/
/*!
  @brief    Start fade power animation.
*/
/******************************************************************************/
void Ledstrip::_powerFade() {
    _state = _POWER_FADE;
    
    _l.logd("Start powerFade mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "PowerHandler",                                                         //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief   Fade power animation thread.
*/
/******************************************************************************/
void Ledstrip::__powerFade() {
    /* Fade */
    float color1Portion = 0;
    
    for (uint16_t timeStep = 0; timeStep <= 100; timeStep++) {
        color1Portion = timeStep * 1.0 / 100;
            
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (_isOn) {
                _leds[i] = _blendColors(_savedLeds[i], color1Portion, CRGB(0, 0, 0));
            } else {
                _leds[i] = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[i]);
            }
        }
        _showLeds();
        vTaskDelay(5);
    }
    
    _l.logd("End powerFade mode");
    
    _state = _READY_TO_RUN;
    
    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Start dissolve power animation.
*/
/******************************************************************************/
void Ledstrip::_powerDissolve() {
    _state = _POWER_DISSOLVE;
    
    _l.logd("Start powerDissolve mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "PowerHandler",                                                         //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Dissolve power animation thread.
*/
/******************************************************************************/
void Ledstrip::__powerDissolve() {
    uint16_t randomIndex = 0;
    uint16_t temp = 0;
    uint16_t indexes[_highestPixelAddress];
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        indexes[i] = i;
    }
        
    /* Shuffle order on randomized index */
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        randomIndex = random(i, _highestPixelAddress);                                      //Generate a random index between i and n, index < i is already randomized
        
        /* Swap elements */
        temp = indexes[i];
        indexes[i] = indexes[randomIndex];
        indexes[randomIndex] = temp;
    }

    /* Draw randomized leds */
    float color1Portion = 0;
    CRGB dotColor;

    for (uint16_t i = 0; i < _highestPixelAddress; i += 3) {
        for (uint16_t timeStep = 0; timeStep <= 10; timeStep++) {
            color1Portion = timeStep * 1.0 / 10;
            
            if (_isOn) {
                dotColor = _blendColors(_savedLeds[indexes[i]], color1Portion, CRGB(0, 0, 0));
            } else {
                dotColor = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[indexes[i]]);
            }

            uint8_t j = 0;
            while (j < 3 && i+j < _highestPixelAddress) {
                _leds[indexes[i+j]] = dotColor;
                j++;
            }
            _showLeds();
        }
    }

    _l.logd("End powerDissolve mode");
    
    _state = _READY_TO_RUN;
    
    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Start sweep power animation.
*/
/******************************************************************************/
void Ledstrip::_powerSweep() {
    _state = _POWER_SWEEP;
    
    _l.logd("Start powerSweep mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "PowerHandler",                                                         //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Sweep power animation thread.
*/
/******************************************************************************/
void Ledstrip::__powerSweep() {
    /* Draw randomized leds */
    float color1Portion = 0;

    uint8_t fadeLength = 5;
    
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        for (uint8_t j = 0; j < fadeLength; j++) {
            color1Portion = (fadeLength-j) * 1.0 / fadeLength;

            if (i + j < _highestPixelAddress) {
                if (_isOn) {
                    _leds[i + j] = _blendColors(_savedLeds[i + j], color1Portion, CRGB(0, 0, 0));
                } else {
                    _leds[i + j] = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[i + j]);
                }
            }
        }
        _showLeds();
        vTaskDelay(50);
    }
  
    _l.logd("End powerSweep mode");
    
    _state = _READY_TO_RUN;

    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Start dual sweep power animation.
*/
/******************************************************************************/
void Ledstrip::_powerDualSweep() {
    _state = _POWER_DUAL_SWEEP;
    
    _l.logd("Start powerDualSweep mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "PowerHandler",                                                         //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Dual sweep power animation thread.
*/
/******************************************************************************/
void Ledstrip::__powerDualSweep() {
    /* Draw randomized leds */
    float color1Portion = 0;

    uint8_t fadeLength = 5;
    
    for (uint16_t i = 0; i < _highestPixelAddress/2; i++) {
        for (uint8_t j = 0; j < fadeLength; j++) {
            color1Portion = (fadeLength-j) * 1.0 / fadeLength;

            if (i + j < _highestPixelAddress/2+1) {
                if (_isOn) {
                    _leds[i + j] = _blendColors(_savedLeds[i + j], color1Portion, CRGB(0, 0, 0));
                } else {
                    _leds[i + j] = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[i + j]);
                }
            }
        }

        for (uint8_t j = 0; j < fadeLength; j++) {
            color1Portion = (fadeLength-j) * 1.0 / fadeLength;

            if (_highestPixelAddress-1 - i - j > _highestPixelAddress/2) {
                if (_isOn) {
                    _leds[_highestPixelAddress-1 - i - j] = _blendColors(_savedLeds[_highestPixelAddress-1 - i - j], color1Portion, CRGB(0, 0, 0));
                } else {
                    _leds[_highestPixelAddress-1 - i - j] = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[_highestPixelAddress-1 - i - j]);
                }
            }
        }
        _showLeds();
        vTaskDelay(50);
    }
    
    /* Make sure every led is fully on/off */
    if (_isOn) {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = _savedLeds[i];
        }
    } else {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = CRGB(0, 0, 0);
        }
    }
    _showLeds();
  
    _l.logd("End powerDualSweep mode");
    
    _state = _READY_TO_RUN;

    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Start multi sweep power animation.
*/
/******************************************************************************/
void Ledstrip::_powerMultiSweep() {
    _state = _POWER_MULTI_SWEEP;
    
    _l.logd("Start powerMultiSweep mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "PowerHandler",                                                         //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Multi sweep power animation thread.
*/
/******************************************************************************/
void Ledstrip::__powerMultiSweep() {
    /* Draw randomized leds */
    float color1Portion = 0;

    uint8_t fadeLength = 5;
    
    for (uint16_t i = 0; i < _highestPixelAddress/2; i++) {
        for (uint8_t j = 0; j < fadeLength; j++) {
            color1Portion = (fadeLength-j) * 1.0 / fadeLength;

            if (i + j < _highestPixelAddress/2+1) {
                if (_isOn) {
                    _leds[i + j] = _blendColors(_savedLeds[i + j], color1Portion, CRGB(0, 0, 0));
                } else {
                    _leds[i + j] = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[i + j]);
                }
            }
        }

        for (uint8_t j = 0; j < fadeLength; j++) {
            color1Portion = (fadeLength-j) * 1.0 / fadeLength;

            if (_highestPixelAddress-1 - i - j > _highestPixelAddress/2) {
                if (_isOn) {
                    _leds[_highestPixelAddress-1 - i - j] = _blendColors(_savedLeds[_highestPixelAddress-1 - i - j], color1Portion, CRGB(0, 0, 0));
                } else {
                    _leds[_highestPixelAddress-1 - i - j] = _blendColors(CRGB(0, 0, 0), color1Portion, _savedLeds[_highestPixelAddress-1 - i - j]);
                }
            }
        }
        _showLeds();
        vTaskDelay(50);
    }
    
    /* Make sure every led is fully on/off */
    if (_isOn) {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = _savedLeds[i];
        }
    } else {
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            _leds[i] = CRGB(0, 0, 0);
        }
    }
    _showLeds();
  
    _l.logd("End powerMultiSweep mode");
    
    _state = _READY_TO_RUN;

    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}
#pragma endregion


#pragma region Utilities
/******************************************************************************/
/*!
  @brief    Rotate LEDs to the left.
  @param    steps               Steps (LEDs) to rotate
*/
/******************************************************************************/
void Ledstrip::_rotateLeft(uint8_t steps) {
    for (uint8_t step = 0; step < steps; step++) {
        CRGB temp = _leds[0];
        for (uint16_t i = 0; i < _highestPixelAddress-1; i++){
            _leds[i] = _leds[i + 1];
        }

        _leds[_highestPixelAddress-1] = temp;
    }
}

/******************************************************************************/
/*!
  @brief    Rotate LEDs to the right.
  @param    steps               Steps (LEDs) to rotate
*/
/******************************************************************************/
void Ledstrip::_rotateRight(uint8_t steps) {
    CRGB temp[steps];                                                           //Temporary array
    
    /* Copying last d elements in the temporary array */
    for (uint16_t i = _highestPixelAddress - steps; i < _highestPixelAddress; i++) {
        temp[i - (_highestPixelAddress - steps)] = _leds[i];
    }

    /* 
     * Shift first (n-d) elements to the right by 
     * d places in the specified array
     */
    for (int16_t i = _highestPixelAddress - steps - 1; i >= 0; i--) {
        _leds[i + steps] = _leds[i];
    }
    
    /* 
     * Place the elements of the temporary array
     * in the first d places of the specified array
     */
    for (uint8_t i = 0; i < steps; i++) {
        _leds[i] = temp[i];
    }
}

/******************************************************************************/
/*!
  @brief    Generates a random CRGB type color.
  @param    saturationPerc      The saturation percentage (0-100)
  @returns  CRGB                The color
*/
/******************************************************************************/
CRGB Ledstrip::_randomColor(uint8_t saturationPerc) {
    uint8_t r = random(0, 255);
    uint8_t g = random(0, 255);
    uint8_t b = random(0, 255);
    uint16_t limit = (uint16_t) 765 - 765 * saturationPerc / 100;               //Calculate limit (for saturation)
  
    if (r + g + b > limit) {                                                    //If color exceeds limit, turn one channel 0
        uint8_t randomRGB = random(0, 2);
        if (randomRGB == 0) {
            r = 0;
        }
        if (randomRGB == 1) {
            g = 0;
        }
        if (randomRGB == 2) {
            b = 0;
        }
    }
    return CRGB (r, g, b);
}

/******************************************************************************/
/*!
  @brief    Set mode based on number.
  @param    mode                Mode to switch to
*/
/******************************************************************************/
CRGB Ledstrip::_blendColors(CRGB color1, float color1Portion, CRGB color2) {
    float portion2 = 1.0 - color1Portion;
    uint8_t r = round(color1[0] * color1Portion * 1.0 + color2[0] * portion2 * 1.0);
    uint8_t g = round(color1[1] * color1Portion * 1.0 + color2[1] * portion2 * 1.0);
    uint8_t b = round(color1[2] * color1Portion * 1.0 + color2[2] * portion2 * 1.0);
    
    return CRGB (r, g, b);
}

/******************************************************************************/
/*!
  @brief    For calculating parallel strip and showing
*/
/******************************************************************************/
void Ledstrip::_showLeds() {
    if (_isOn || _state < NUM_POWER_ANIMATIONS) {
        if (_driver == _SK6812) {
            for (uint8_t i = 0; i < _numberLeds; i++) {
                _crgbwTempLeds[i] = CRGBtoCRGBW(_leds[_ledAddresses[i]]);
            }
        } else {
            for (uint8_t i = 0; i < _numberLeds; i++) {
                _tempLeds[i] = _leds[_ledAddresses[i]];
            }
        }
        FastLED.show();
    } else {
        _l.logd("Leds not updated because the strip is off");
    }
}

/******************************************************************************/
/*!
  @brief    Used to pick colors for rainbow method.
  @param    position            Position on wheel (0-255)
  @returns  CRGB color          The color
*/
/******************************************************************************/
CRGB Ledstrip::_colorWheel(uint8_t position) {
    if (position < 85) {
        return CRGB (position * 3, 255 - position * 3, 0);
    } else if (position < 170) {
        position -= 85;
        return CRGB (255 - position * 3, 0, position * 3);
    } else {
        position -= 170;
        return CRGB (0, position * 3, 255 - position * 3);
    }
}

/******************************************************************************/
/*!
  @brief    Function to start the different threads.
*/
/******************************************************************************/
void Ledstrip::__startModeTask(void* parameter) {
    Ledstrip* ledRef = static_cast<Ledstrip *>(parameter);

    switch (ledRef->_state) {
        case _POWER_FADE:
            ledRef->__powerFade();
            break;
        case _POWER_DISSOLVE:
            ledRef->__powerDissolve();
            break;
        case _POWER_SWEEP:
            ledRef->__powerSweep();
            break;
        case _POWER_DUAL_SWEEP:
            ledRef->__powerDualSweep();
            break;
        case _POWER_MULTI_SWEEP:
            ledRef->__powerMultiSweep();
            break;

        case _FADE_BRIGHTNESS:
            ledRef->__fadeBrightness();
            break;
        case _FADE_TO_SINGLE_COLOR:
            ledRef->__fadeToColor();
            break;
        case _FADE_TO_MULTIPLE_COLOR:
            ledRef->__fadeToMultipleColors();
            break;
        default:
            break;
    }

    if (ledRef->_state == _LOOPING) {
        switch (ledRef->_mode) {
            case MODE_FADE:
                ledRef->__fade();
                break;
            case MODE_GRADIENT:
                ledRef->__gradient();
                break;
            case MODE_BLINK:
                ledRef->__blink();
                break;
            case MODE_SCAN:
                ledRef->__scan();
                break;
            case MODE_THEATER:
                ledRef->__theater();
                break;
            case MODE_SINE:
                ledRef->__sine();
                break;
            case MODE_BOUNCING_BALLS:
                ledRef->__bouncingBalls();
                break;
            case MODE_DISSOLVE:
                ledRef->__dissolve();
                break;
            case MODE_SPARKLE:
                ledRef->__sparkle();
                break;
            case MODE_FIREWORKS:
                ledRef->__fireworks();
                break;
            case MODE_FIRE:
                ledRef->__fire();
                break;
            case MODE_SWEEP:
                ledRef->__sweep();
                break;
            case MODE_COLOR_TWINKELS:
                ledRef->__colorTwinkels();
                break;
            case MODE_METEOR_RAIN:
                ledRef->__meteorRain();
                break;
            case MODE_COLOR_WAVES:
                ledRef->__colorWaves();
                break;
            case MODE_TEMPLATE_1:
                ledRef->__modeTemplate1();
                break;
            case MODE_TEMPLATE_2:
                ledRef->__modeTemplate2();
                break;
            case MODE_TEMPLATE_3:
                ledRef->__modeTemplate3();
                break;
            case MODE_TEMPLATE_4:
                ledRef->__modeTemplate4();
                break;
            case MODE_TEMPLATE_5:
                ledRef->__modeTemplate5();
                break;
            case MODE_TEMPLATE_6:
                ledRef->__modeTemplate6();
                break;
            case MODE_TEMPLATE_7:
                ledRef->__modeTemplate7();
                break;
            case MODE_TEMPLATE_8:
                ledRef->__modeTemplate8();
                break;
            case MODE_TEMPLATE_9:
                ledRef->__modeTemplate9();
                break;
            case MODE_TEMPLATE_10:
                ledRef->__modeTemplate10();
                break;

            case SYSTEM_MODE_PULSES:
                ledRef->__systemPulses();
                break;
            case SYSTEM_MODE_ALARM:
                ledRef->__systemAlarm();
                break;
            default:
                break;
        }
    }
}

/******************************************************************************/
/*!
  @brief    Wait until strip is finished critical system tasks.
*/
/******************************************************************************/
void Ledstrip::_waitUntilIdle() {
    if (_state == _LOOPING) {
        vTaskDelete(_taskHandler);
        _taskHandler = NULL;
        _l.logd("Ended looping mode");
        
        vTaskDelay(10);                                                          //Otherwise program gets stuck
        _state = _READY_TO_RUN;
    }
    
    while (_state != _READY_TO_RUN && _state != _WAIT_FOR_DOOR_CLOSED) {
        vTaskDelay(1);
    }
}

/******************************************************************************/
/*!
  @brief    Saves the current led states of the strip.
*/
/******************************************************************************/
void Ledstrip::_saveLeds() {
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        _savedLeds[i] = _leds[i];
    }
}

/******************************************************************************/
/*!
  @brief    Saves the current led states of the strip.
*/
/******************************************************************************/
void Ledstrip::_loadPixelAddresses() {
    _nvMemory.begin(NV_MEM_CONFIG);
    String addressString = _nvMemory.getString("ledAddresses");
    _nvMemory.end();
    
    if (addressString == "") {
        for (uint16_t i = 0; i < _numberLeds; i++) {
            _ledAddresses[i] = i;
        }
        _highestPixelAddress = _numberLeds;
        return;
    }
        
    JsonDocument jsonParser;
    deserializeJson(jsonParser, addressString);  //Convert JSON string to object
    
    _highestPixelAddress = 0;
    for (uint16_t i = 0; i < _numberLeds; i++) {
        _ledAddresses[i] = (uint16_t) jsonParser[i];
        if (_highestPixelAddress < _ledAddresses[i]) {
            _highestPixelAddress = _ledAddresses[i];
        }
    }
    _highestPixelAddress += 1;
}

/******************************************************************************/
/*!
  @brief    Calculates the heat color for the specified temperature and
            pallete.
  @param    temperature         Color temperature
  @param    palette             Color palette
  @returns  CRGB color          The color
*/
/******************************************************************************/
CRGB Ledstrip::_getHeatColor(uint8_t temperature, uint8_t palette) {
    /* Scale 'heat' down from 0-255 to 0-191 */
    uint8_t t192 = round((temperature/255.0) * 191);
   
    /* Calculate ramp up */
    uint8_t heatramp = t192 & 0x3F;                                             //0..63
    heatramp <<= 2;                                                             //scale up to 0..252

    if (palette == PALETTE_YELLOW_RED) {
        if (t192 > 0x80) {               
            return CRGB(255, 255, heatramp);                                    //Hottest
        } else if (t192 > 0x40) {          
            return CRGB(255, heatramp, 0);                                      //Middle
        }
        return CRGB(heatramp, 0, 0);                                            //Coolest
    }

    if (palette == PALETTE_PURPLE_BLUE) {
        if (t192 > 0x80) {
            return CRGB(255, heatramp, 255);                                    //Hottest
        } else if (t192 > 0x40) {
            return CRGB(heatramp, 0, 255);                                      //Middle
        }
        return CRGB(0, 0, heatramp);                                            //Coolest
    }

    if (palette == PALETTE_GREEN_BLUE) {
        if (t192 > 0x80) {
            return CRGB(heatramp, 255, 255);                                    //Hottest
        } else if (t192 > 0x40) {
            return CRGB(0, heatramp, 255);                                      //Middle
        }
        return CRGB(0, 0, heatramp);                                            //Coolest
    }

    if (palette == PALETTE_BLUE_GREEN) {
        if (t192 > 0x80) {
            return CRGB(heatramp, 255, 255);                                    //Hottest
        } else if (t192 > 0x40) {
            return CRGB(0, 255, heatramp);                                      //Middle
        }
        return CRGB(0, heatramp, 0);                                            //Coolest
    }
}

/******************************************************************************/
/*!
  @brief    Calculates the color position of the gradient mode.
  @param    steps               Steps (LEDs) of loop
  @returns  uint8_t             Color position
*/
/******************************************************************************/
uint8_t Ledstrip::_getGradientColorPosition(uint8_t step) {
    uint8_t colorMultiplier = MAX_WAVE_LENGTH+1 - _modeParameters[MODE_GRADIENT].waveLength;
    uint8_t range = _modeParameters[MODE_GRADIENT].maxColorPos - _modeParameters[MODE_GRADIENT].minColorPos;
    if (range == 0) range++;

    uint8_t colorPosition = (step * colorMultiplier + _modeParameters[MODE_GRADIENT].colorPosition) & 255;

    if (colorPosition < _modeParameters[MODE_GRADIENT].minColorPos) {
        uint8_t diff = _modeParameters[MODE_GRADIENT].minColorPos - colorPosition;
        colorPosition = _modeParameters[MODE_GRADIENT].minColorPos + (diff % (2 * range));
        if (colorPosition > _modeParameters[MODE_GRADIENT].maxColorPos) {
            colorPosition = _modeParameters[MODE_GRADIENT].maxColorPos - (colorPosition - _modeParameters[MODE_GRADIENT].maxColorPos);
        }
    } else if (colorPosition > _modeParameters[MODE_GRADIENT].maxColorPos) {
        uint8_t diff = colorPosition - _modeParameters[MODE_GRADIENT].maxColorPos;
        colorPosition = _modeParameters[MODE_GRADIENT].maxColorPos - (diff % (2 * range));
        if (colorPosition < _modeParameters[MODE_GRADIENT].minColorPos) {
            colorPosition = _modeParameters[MODE_GRADIENT].minColorPos + (_modeParameters[MODE_GRADIENT].minColorPos - colorPosition);
        }
    }
}
#pragma endregion

#pragma region Getters
/******************************************************************************/
/*!
  @brief    Returns the state of the ledstrip.
  @returns  uint8_t             State of the strip
*/
/******************************************************************************/
uint8_t Ledstrip::getState() {
    return _state;
}

/******************************************************************************/
/*!
  @brief    Returns the number of LEDs in the ledstrip.
  @returns  uint8_t             Number of LEDs
*/
/******************************************************************************/
uint16_t Ledstrip::getNumberOfLeds() {
    return _numberLeds;
}

/******************************************************************************/
/*!
  @brief    Returns the driver of the ledstrip.
  @returns  uint8_t             Strip driver ID
*/
/******************************************************************************/
uint8_t Ledstrip::getDriver() {
    return _driver;
}

/******************************************************************************/
/*!
  @brief    Returns whether the ledstrip is available.
  @returns  bool                True if strip is available for commands
*/
/******************************************************************************/
bool Ledstrip::isAvailable() {
    if (_state == _READY_TO_RUN) {
        return true;
    }
  
    if (_state == _LOOPING) {
        return true;
    }
  
    if (_state == _WAIT_FOR_DOOR_CLOSED) {
        return true;
    }
    return false;
}

/******************************************************************************/
/*!
  @brief    Returns the pixel addressing as JSON string.
  @returns  String              JSON string of pixel addressing
*/
/******************************************************************************/
String Ledstrip::getPixelAddressing() {
    _nvMemory.begin(NV_MEM_CONFIG);
    String addresses = _nvMemory.getString("ledAddresses");
    _nvMemory.end();

    return addresses;
}

/******************************************************************************/
/*!
  @brief    Returns a list of pixel values. 0 when is off 1 when is on.
  @returns  String              JSON string of pixel values
*/
/******************************************************************************/
String Ledstrip::getPixels() {
    String jsonString = "[";
    
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        if (_leds[i] == CRGB (0,0,0)) {
            jsonString += "1";
        } else {
            jsonString += "0";
        }

        if (i < _highestPixelAddress-1) {
            jsonString += ", ";
        }
    }
    jsonString += "]";
}

/******************************************************************************/
/*!
  @brief    Returns the power state of the ledstrip.
  @returns  bool                True if strip is on.
*/
/******************************************************************************/
bool Ledstrip::getPower() {
    return _isOn;
}

/******************************************************************************/
/*!
  @brief    Returns the mode of the ledstrip.
  @returns  uint8_t             Mode ID
*/
/******************************************************************************/
uint8_t Ledstrip::getMode() {
    return _mode;
}

/******************************************************************************/
/*!
  @brief    Returns the power animation of the ledstrip.
  @returns  uint8_t             Power animation
*/
/******************************************************************************/
uint8_t Ledstrip::getPowerAnimation() {
    return _powerAnimation;
}

/******************************************************************************/
/*!
  @brief    Returns the brightness of the ledstrip.
  @returns  uint8_t             Brightness
*/
/******************************************************************************/
uint8_t Ledstrip::getBrightness() {
    return _brightness;
}
#pragma endregion

#pragma region Setters
/******************************************************************************/
/*!
  @brief    Sets the brightness of the strip.
  @param    brightness          Brightness to set
*/
/******************************************************************************/
void Ledstrip::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    _fadeBrightness();
}
#pragma endregion

#pragma region Faders
/******************************************************************************/
/*!
  @brief    Fade to the specified brightness.
*/
/******************************************************************************/
void Ledstrip::_fadeBrightness() {
    if (_brightness == FastLED.getBrightness()) {
        return;
    }

    _waitUntilIdle();
        
    _state = _FADE_BRIGHTNESS;
    
    _l.logd("Start fadeBrightness mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "FadeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Fade to the specified brightness.
*/
/******************************************************************************/
void Ledstrip::__fadeBrightness() {
    uint8_t currBrightness = FastLED.getBrightness();
    int8_t dir = 0;

    /* Calculate if current brightness is greater or smaller than the specified brightness */
    if (currBrightness < _brightness) {
        dir = 1;
    } else if (currBrightness > _brightness) {
        dir = -1;
    }

    while (currBrightness != _brightness) {
        currBrightness += dir;
        FastLED.setBrightness(currBrightness);
        FastLED.show();

        if (currBrightness == 0 || currBrightness == MAX_BRIGHTNESS) {
            break;
        }
        vTaskDelay(BRIGHTNESS_DELAY);
    }

    _l.logd("End fadeBrightness mode");
    _state = _READY_TO_RUN;

    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Fade from multiple colors to one single color.
*/
/******************************************************************************/
void Ledstrip::_fadeToColor() {
    _waitUntilIdle();

    _state = _FADE_TO_SINGLE_COLOR;
    
    _l.logd("Start fadeToColor mode");
        
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "FadeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Fade from multiple colors to one single color.
*/
/******************************************************************************/
void Ledstrip::__fadeToColor() {
    int8_t directions[_highestPixelAddress][3] = {0};                           //For colorshifting, rgb
    uint16_t numDone = 0;
    
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        for (uint8_t colorChan = 0; colorChan < 3; colorChan++) {               //Set directions for slowly shifting to right number
            if (_fullColor[colorChan] < _leds[i][colorChan]) {
                directions[i][colorChan] = -1;
            } else if (_fullColor[colorChan] > _leds[i][colorChan]) {
                directions[i][colorChan] = 1;
            }
        }
    }
    
    while (numDone < _highestPixelAddress) {
        numDone = 0;
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (_fullColor[0] == _leds[i][0] && _fullColor[1] == _leds[i][1] && _fullColor[2] == _leds[i][2]) {
                numDone++;
            }
             
            /* If color is not the specified color, shift rgb numbers towards color */
            for (uint8_t colorChan = 0; colorChan < 3; colorChan++) {
                if (_fullColor[colorChan] != _leds[i][colorChan]) {
                    _leds[i][colorChan] += directions[i][colorChan];
                }
            }
        }
        _showLeds();
        vTaskDelay(COLOR_DELAY);
    }

    _l.logd("End fadeToColor mode");
    
    _state = _READY_TO_RUN;

    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Fade from one single color to multiple colors.
  @param    desiredColorPos         Color position to fade to
  @param    fadeToGradientColors    When true, the gradient wavelength etc. is
                                    used
*/
/******************************************************************************/
void Ledstrip::_fadeToMultipleColors(uint8_t desiredColorPos, bool fadeToGradientColors) {
    _waitUntilIdle();

    _desiredColorPos = desiredColorPos;
    _fadeToGradientColors = desiredColorPos;

    _state = _FADE_TO_MULTIPLE_COLOR;
    
    _l.logd("Start fadeToMultipleColors mode");
    
    xTaskCreatePinnedToCore(
        Ledstrip::__startModeTask,                                              //Task function
        "FadeHandler",                                                          //Task name
        8000,                                                                   //Stack size in bytes
        this,                                                                   //Task parameter
        PRIORITY,                                                               //Task priority
        &_taskHandler,                                                          //Task handler
        CORE_NUMBER                                                             //Task CPU core
    );
}

/******************************************************************************/
/*!
  @brief    Fade from one single color to multiple colors.
*/
/******************************************************************************/
void Ledstrip::__fadeToMultipleColors() {
    CRGB desiredColors[_highestPixelAddress];
    int8_t directions[_highestPixelAddress][3] = {0};                           //For colorshifting, rgb
    uint16_t numDone = 0;

    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        if (_fadeToGradientColors) {
            uint8_t colorPosition = _getGradientColorPosition(i);
            desiredColors[i] = _colorWheel(colorPosition);
            desiredColors[_highestPixelAddress-1 - i] = _colorWheel(colorPosition);
            continue;
        }

        desiredColors[i] = _colorWheel((i + _desiredColorPos) & 255);
        desiredColors[_highestPixelAddress-1 - i] = _colorWheel((i + _desiredColorPos) & 255);
    }
        
    for (uint16_t i = 0; i < _highestPixelAddress; i++) {
        for (uint8_t colorChan = 0; colorChan < 3; colorChan++) {               //Set directions for slowly shifting to right number
            if (desiredColors[i][colorChan] < _leds[i][colorChan]) {
                directions[i][colorChan] = -1;
            } else if (desiredColors[i][colorChan] > _leds[i][colorChan]) {
                directions[i][colorChan] = 1;
            }
        }
    }
    
    while (numDone < _highestPixelAddress) {
        numDone = 0;
        for (uint16_t i = 0; i < _highestPixelAddress; i++) {
            if (desiredColors[i][0] == _leds[i][0] && desiredColors[i][1] == _leds[i][1] && desiredColors[i][2] == _leds[i][2]) {
                numDone++;
            }

            for (uint8_t colorChan = 0; colorChan < 3; colorChan++) {
                if (desiredColors[i][colorChan] != _leds[i][colorChan]) {
                    _leds[i][colorChan] += directions[i][colorChan];
                }
            }
        }
        _showLeds();
        vTaskDelay(COLOR_DELAY);
    }
    
    _l.logd("End fadeToMultipleColors mode");
    
    _state = _READY_TO_RUN;

    _taskHandler = NULL;
    vTaskDelete(_taskHandler);
}

/******************************************************************************/
/*!
  @brief    Converts the specified CRGB color into CRGBW.
  @param    color               CRGB color
  @param    CRGBW               CRGBW color
*/
/******************************************************************************/
CRGBW Ledstrip::CRGBtoCRGBW(CRGB color) {
    return CRGBW(color.r, color.g, color.b, 0);
}
#pragma endregion