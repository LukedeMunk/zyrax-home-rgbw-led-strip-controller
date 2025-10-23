/******************************************************************************/
/*
 * File:    ZyraX_Home_RGBW_ledstrip_controller.ino
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Code for an addressable RGBW LED strip. Needs ZyraX Home main
 *          controller for controlling. Handles HTTP commands, WiFi connection,
 *          configuring the LED strip and sensor state changes.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "Ledstrip.h"                                                           //For LED strip functionality
#include "HTTPClient.h"                                                         //To make requests to master controller
#include "CommandQueue.h"                                                       //For asynchronous execution of commands
#include "MemoryManager.h"                                                      //For managing files on the SD card
#include "UpdateManager.h"                                                      //For updating the firmware
#include "esp_mac.h"                                                            //For generating a unique network config AP name based on MAC
#include "Ticker.h"                                                             //For asynchronous delays and intervals
#include "ArduinoJson.h"                                                        //For JSON functionality
#include "SecurityManager.h"                                                    //For encrypting/decrypting and hashing functionality

/* Webserver and network */
#include "WiFi.h"                                                               //For WiFi functionality
#include "ESPAsyncWebServer.h"                                                  //For HTTP commands
#include "SPIFFS.h"                                                             //For file interaction with internal SPI Flash File System

#include "Configuration.h"                                                      //For configuration variables and global constants
#include "Logger.h"                                                             //For printing and saving logs

#pragma region Global variables
/* For sensor */
bool localDoorState = false;
bool doorStateChanged = false;

Ticker rebootDelay;

AsyncWebServer server(80);
Ledstrip strip;

TaskHandle_t _taskHandler = NULL;

CommandQueue commandQueue;

Logger l("Main");
MemoryManager memoryManager;
UpdateManager updateManager;
Preferences nvMemory;
NetworkConfiguration networkConfig;
SecurityManager securityManager;

uint8_t id;
bool sensorEnabled;
bool sensorInverted;
uint8_t sensorModel;

uint8_t bootMode = BOOT_MODE_NORMAL;
#pragma endregion

#pragma region Function declarations with default parameters
bool checkParameters(AsyncWebServerRequest *request, const char* neededParameters[], uint8_t numberOfParameters, bool needsAll = true, bool post = false);
bool checkPostParameters(AsyncWebServerRequest *request, const char* neededParameters[], uint8_t numberOfParameters, bool needsAll = true);
String generateResponseJson(String endpoint, uint16_t responseCode, String message = "");
#pragma endregion


/******************************************************************************/
/*!
  @brief    Sets up the controller. Loads configuration and initializes
            components.
*/
/******************************************************************************/
void setup() {
    Serial.begin(115200);
    delay(100);

    memoryManager.initialize();
    
    nvMemory.begin(NV_MEM_CONFIG, true);
    id = nvMemory.getUChar("id", DEFAULT_ID);
    sensorEnabled = nvMemory.getBool("sensorEnabled", false);
    sensorInverted = nvMemory.getBool("sensorInverted", false);
    sensorModel = nvMemory.getUChar("sensorModel", SENSOR_MODEL_CONTACT_SWITCH);
    nvMemory.end();
    l.logi("id: " + String(id));

    loadNetworkConfiguration();

    countNumberOfLogs();
    logResetReason();
    initializeWifi();

    strip.initialize();

    if (bootMode == BOOT_MODE_NETWORK_CONFIGURATION) {
        Command command;
        command.command = COMMAND_SET_MODE;
        command.parameter1 = SYSTEM_MODE_PULSES;
        commandQueue.pushCommand(command);
        command.command = COMMAND_SET_POWER;
        command.parameter1 = 1;
        commandQueue.pushCommand(command);
    } else {
        if (strip.getMode() == SYSTEM_MODE_PULSES) {
            strip.setMode(MODE_COLOR);
        }
        updateManager.initialize(id);
    }

    createCommandRoutes();
    server.begin();

    l.logi("Started");
    connectToMasterServer();
    
    if (sensorEnabled) {
        initializeSensor();
    }

    xTaskCreatePinnedToCore(
        executeCommands,                                                        //Task function
        "executeCommand",                                                       //String with name of task (by default max 16 characters long) 
        8000,                                                                   //Stack size in bytes
        NULL,                                                                   //Parameter passed as input of the task
        0,                                                                      //Priority of the task
        &_taskHandler,                                                          //Task handler
        0
    );
}


/******************************************************************************/
/*!
  @brief    Initializes the local wired sensor.
*/
/******************************************************************************/
void initializeSensor() {
    if (sensorModel == SENSOR_MODEL_CONTACT_SWITCH) {
        pinMode(LOCAL_SENSOR1_PIN, INPUT_PULLUP);
        pinMode(LOCAL_SENSOR2_PIN, INPUT_PULLUP);
    }
}

/******************************************************************************/
/*!
  @brief    Mainloop.
*/
/******************************************************************************/
void loop() {
    if (sensorEnabled) {
        checkSensor();
    }
}

#pragma region Endpoint functions
/******************************************************************************/
/*!
  @brief    Handles HTTP request. Returns the log file.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void downloadLogs(AsyncWebServerRequest *request) {
    String path = memoryManager.joinPaths(LOGS_DIRECTORY, LOGS_FILE);

    if (!SD_MMC.exists(path)) {
        String resultString = generateResponseJson(request->url(), HTTP_CODE_NOT_FOUND, "File does not exist");
        request->send(HTTP_CODE_NOT_FOUND, "text/javascript", resultString);
        l.logw("File does not exist, path: " + path);
        return;
    }

    request->send(SD_MMC, path, "application/json");
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Configures the controller configuration
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void setConfiguration(AsyncWebServerRequest *request) {
    const char* neededParameters[] = {
                                        "id",
                                        "hostname",
                                        "power_animation",
                                        "model_id",
                                        "led_addresses",
                                        "number_of_leds",
                                        "has_sensor",
                                        "sensor_inverted",
                                        "sensor_model"
                                    };

    if (!checkPostParameters(request, neededParameters, 9, false)) {
        return;
    }

    String resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "application/json", resultString);
    
    bool needsRestart = false;

    if (request->hasParam("hostname", true)) {
        l.logd("hostname: " + request->getParam("hostname", true)->value());
        String hostname = request->getParam("hostname", true)->value();
        if (hostname != networkConfig.hostname) {
            networkConfig.hostname = hostname;
            nvMemory.begin(NV_MEM_CONFIG);
            nvMemory.putString("hostname", hostname);
            nvMemory.end();
            needsRestart = true;
        }
    }

    if (request->hasParam("id", true)) {
        l.logd("id: " + request->getParam("id", true)->value());
        uint8_t webId = (uint8_t) atoi(request->getParam("id", true)->value().c_str());
        if (webId != id) {
            id = webId;
            nvMemory.begin(NV_MEM_CONFIG);
            nvMemory.putUChar("id", id);
            nvMemory.end();
        }
    }

    if (request->hasParam("power_animation", true)) {
        l.logd("power_animation: " + request->getParam("power_animation", true)->value());
        uint8_t powerAnimation = (uint8_t) atoi(request->getParam("power_animation", true)->value().c_str());
        strip.setPowerAnimation(powerAnimation);
    }

    if (request->hasParam("has_sensor", true)) {
        l.logd("has_sensor: " + request->getParam("has_sensor", true)->value());
        bool enabled = (bool) atoi(request->getParam("has_sensor", true)->value().c_str());
        if (enabled != sensorEnabled) {
            sensorEnabled = enabled;
            nvMemory.begin(NV_MEM_CONFIG);
            nvMemory.putBool("sensorEnabled", enabled);
            nvMemory.end();
        }
    }

    if (request->hasParam("sensor_inverted", true)) {
        l.logd("sensor_inverted: " + request->getParam("sensor_inverted", true)->value());
        bool inverted = (bool) atoi(request->getParam("sensor_inverted", true)->value().c_str());
        if (inverted != sensorInverted) {
            sensorInverted = inverted;
            nvMemory.begin(NV_MEM_CONFIG);
            nvMemory.putBool("sensorInverted", inverted);
            nvMemory.end();
        }
    }

    if (request->hasParam("sensor_model", true)) {
        l.logd("sensor_model: " + request->getParam("sensor_model", true)->value());
        uint8_t model = (uint8_t) atoi(request->getParam("sensor_model", true)->value().c_str());
        if (model != sensorModel) {
            nvMemory.begin(NV_MEM_CONFIG);
            nvMemory.putUChar("sensorModel", model);
            nvMemory.end();
            needsRestart = true;
        }
    }

    if (request->hasParam("model_id", true)) {
        l.logd("driver: " + request->getParam("model_id", true)->value());
        uint8_t driver = (uint8_t) atoi(request->getParam("model_id", true)->value().c_str());
        if (driver != strip.getDriver()) {
            nvMemory.begin(NV_MEM_CONFIG);
            nvMemory.putUChar("driver", driver);
            nvMemory.end();
            needsRestart = true;
        }
    }

    if (request->hasParam("led_addresses", true)) {
        String addresses = request->getParam("led_addresses", true)->value();
        uint16_t numberOfLeds = (uint16_t) atoi(request->getParam("number_of_leds", true)->value().c_str());
        l.logd("led_addresses: " + addresses);

        if (numberOfLeds == 0) {
            l.logw("Number of leds cannot be zero, ignoring");
        } else if (numberOfLeds >= MAX_NUMBER_LEDS) {
            l.logw("Number of leds too high, ignoring");
        } else {
            //if (numberOfLeds != strip.getNumberOfLeds()) {
            //    needsRestart = true;
            //}
            strip.setPixelAddressing(addresses, numberOfLeds);
        }
    }

    if (needsRestart) {
        rebootDelay.once(1, rebootTicker);                                      //Reboot delay and return for HTTP to return response
    }
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Deletes the logs.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void deleteLogs(AsyncWebServerRequest *request) {
    String resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "application/json", resultString);

    l.markLogsAsRead();
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Sets the LED power.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void setPower(AsyncWebServerRequest *request) {
    String resultString;
    const char* neededParameters[] = {"power"};

    if (!checkPostParameters(request, neededParameters, 1)) {
        return;
    }

    Command command;
    command.command = COMMAND_SET_POWER;
    command.parameter1 = (uint8_t) atoi(request->getParam("power", true)->value().c_str());

    if (commandQueue.pushCommand(command)) {
        resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
        request->send(HTTP_CODE_OK, "application/json", resultString);
    } else {
        resultString = generateResponseJson(request->url(), HTTP_CODE_SERVICE_UNAVAILABLE, "Not available");
        request->send(HTTP_CODE_SERVICE_UNAVAILABLE, "application/json", resultString);
    }
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Sets the indoor LED brightness.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void setBrightness(AsyncWebServerRequest *request) {
    String resultString;
    const char* neededParameters[] = {"brightness"};

    if (!checkPostParameters(request, neededParameters, 1)) {
        return;
    }

    Command command;
    command.command = COMMAND_SET_BRIGHTNESS;
    command.parameter1 = (uint8_t) atoi(request->getParam("brightness", true)->value().c_str());

    if (commandQueue.pushCommand(command)) {
        resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
        request->send(HTTP_CODE_OK, "application/json", resultString);
        nvMemory.begin(NV_MEM_CONFIG);
        nvMemory.putUChar("brightness", command.parameter1);
        nvMemory.end();
    } else {
        resultString = generateResponseJson(request->url(), HTTP_CODE_SERVICE_UNAVAILABLE, "Not available");
        request->send(HTTP_CODE_SERVICE_UNAVAILABLE, "application/json", resultString);
    }
    
    command.command = COMMAND_SET_MODE;
    command.parameter1 = strip.getMode();
    commandQueue.pushCommand(command);
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Draws the specified LEDs.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void drawPixels(AsyncWebServerRequest *request) {
    String resultString;
    const char* neededParameters[] = {"leds"};                                  //Leds JSON with HEX colors

    if (!checkPostParameters(request, neededParameters, 1)) {
        return;
    }

    if (!commandQueue.isEmpty() || strip.getMode() != MODE_DRAWING) {
        resultString = generateResponseJson(request->url(), HTTP_CODE_SERVICE_UNAVAILABLE, "Not in drawing mode");
        request->send(HTTP_CODE_SERVICE_UNAVAILABLE, "application/json", resultString);
        return;
    }
    
    JsonDocument jsonParser;

    deserializeJson(jsonParser, request->getParam("leds", true)->value());  //Convert JSON string to object

    CRGB leds[strip.getNumberOfLeds()];
    for (uint16_t i = 0; i < strip.getNumberOfLeds(); i++) {
        leds[i] = hexStringToRGB(jsonParser[i]);
    }

    resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "application/json", resultString);

    strip.drawPixels(leds);
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Sets the animation mode.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void setMode(AsyncWebServerRequest *request) {
    String resultString;
    const char* neededParameters[] = {"mode"};

    if (!checkPostParameters(request, neededParameters, 1)) {
        return;
    }

    Command command;
    command.command = COMMAND_SET_MODE;
    command.parameter1 = (uint8_t) atoi(request->getParam("mode", true)->value().c_str());

    if (commandQueue.pushCommand(command)) {
        resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
        request->send(HTTP_CODE_OK, "application/json", resultString);
    } else {
        resultString = generateResponseJson(request->url(), HTTP_CODE_SERVICE_UNAVAILABLE, "Not available");
        request->send(HTTP_CODE_SERVICE_UNAVAILABLE, "application/json", resultString);
    }
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Configures the mode parameters.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void configureMode(AsyncWebServerRequest *request) {
    String resultString;
    const char* neededParameters[] = {"mode"};

    if (!checkPostParameters(request, neededParameters, 1)) {
        return;
    }

    resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "application/json", resultString);

    uint8_t numberOfParams = request->params();
    l.logd("numberOfParams: " + String(numberOfParams));
    
    uint8_t mode = (uint8_t) atoi(request->getParam("mode", true)->value().c_str());

    l.logd("Mode: " + String(mode));

    ModeParameters parameters;
    
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_MIN_COLOR_POS)) {
        if (!request->hasParam(PARAMETER_NAME_MIN_COLOR_POS, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_MIN_COLOR_POS));
        } else {
            parameters.minColorPos = (uint8_t) atoi(request->getParam(PARAMETER_NAME_MIN_COLOR_POS, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_MAX_COLOR_POS)) {
        if (!request->hasParam(PARAMETER_NAME_MAX_COLOR_POS, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_MAX_COLOR_POS));
        } else {
            parameters.maxColorPos = (uint8_t) atoi(request->getParam(PARAMETER_NAME_MAX_COLOR_POS, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_COLOR1)) {
        if (!request->hasParam(PARAMETER_NAME_COLOR1, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_COLOR1));
        } else {
            parameters.color1 = hexStringToRGB(request->getParam(PARAMETER_NAME_COLOR1, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_COLOR2)) {
        if (!request->hasParam(PARAMETER_NAME_COLOR2, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_COLOR2));
        } else {
            parameters.color2 = hexStringToRGB(request->getParam(PARAMETER_NAME_COLOR2, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_USE_GRADIENT1)) {
        if (!request->hasParam(PARAMETER_NAME_USE_GRADIENT1, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_USE_GRADIENT1));
        } else {
            parameters.useGradient1 = (bool) atoi(request->getParam(PARAMETER_NAME_USE_GRADIENT1, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_USE_GRADIENT2)) {
        if (!request->hasParam(PARAMETER_NAME_USE_GRADIENT2, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_USE_GRADIENT2));
        } else {
            parameters.useGradient2 = (bool) atoi(request->getParam(PARAMETER_NAME_USE_GRADIENT2, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_SEGMENT_SIZE)) {
        if (!request->hasParam(PARAMETER_NAME_SEGMENT_SIZE, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_SEGMENT_SIZE));
        } else {
            parameters.segmentSize = (uint8_t) atoi(request->getParam(PARAMETER_NAME_SEGMENT_SIZE, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_TAIL_LENGTH)) {
        if (!request->hasParam(PARAMETER_NAME_TAIL_LENGTH, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_TAIL_LENGTH));
        } else {
            parameters.tailLength = (uint8_t) atoi(request->getParam(PARAMETER_NAME_TAIL_LENGTH, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_WAVE_LENGTH)) {
        if (!request->hasParam(PARAMETER_NAME_WAVE_LENGTH, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_WAVE_LENGTH));
        } else {
            parameters.waveLength = (uint8_t) atoi(request->getParam(PARAMETER_NAME_WAVE_LENGTH, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_TIME_FADE)) {
        if (!request->hasParam(PARAMETER_NAME_TIME_FADE, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_TIME_FADE));
        } else {
            parameters.timeFade = (uint16_t) atoi(request->getParam(PARAMETER_NAME_TIME_FADE, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_DELAY)) {
        if (!request->hasParam(PARAMETER_NAME_DELAY, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_DELAY));
        } else {
            parameters.delay = (uint16_t) atoi(request->getParam(PARAMETER_NAME_DELAY, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_DELAY_BETWEEN)) {
        if (!request->hasParam(PARAMETER_NAME_DELAY_BETWEEN, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_DELAY_BETWEEN));
        } else {
            parameters.delayBetween = (uint16_t) atoi(request->getParam(PARAMETER_NAME_DELAY_BETWEEN, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_RANDOMNESS_DELAY)) {
        if (!request->hasParam(PARAMETER_NAME_RANDOMNESS_DELAY, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_RANDOMNESS_DELAY));
        } else {
            parameters.randomnessDelay = (uint8_t) atoi(request->getParam(PARAMETER_NAME_RANDOMNESS_DELAY, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_INTENSITY)) {
        if (!request->hasParam(PARAMETER_NAME_INTENSITY, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_INTENSITY));
        } else {
            parameters.intensity = (uint8_t) atoi(request->getParam(PARAMETER_NAME_INTENSITY, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_DIRECTION)) {
        if (!request->hasParam(PARAMETER_NAME_DIRECTION, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_DIRECTION));
        } else {
            parameters.direction = (uint8_t) atoi(request->getParam(PARAMETER_NAME_DIRECTION, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_NUMBER_OF_ELEMENTS)) {
        if (!request->hasParam(PARAMETER_NAME_NUMBER_OF_ELEMENTS, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_NUMBER_OF_ELEMENTS));
        } else {
            parameters.numberOfElements = (uint8_t) atoi(request->getParam(PARAMETER_NAME_NUMBER_OF_ELEMENTS, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_PALETTE)) {
        if (!request->hasParam(PARAMETER_NAME_PALETTE, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_PALETTE));
        } else {
            parameters.palette = (uint8_t) atoi(request->getParam(PARAMETER_NAME_PALETTE, true)->value().c_str());
        }
    }
    if (memoryManager.modeHasParameter(mode, PARAMETER_NAME_FADE_LENGTH)) {
        if (!request->hasParam(PARAMETER_NAME_FADE_LENGTH, true)) {
            l.logw("Missing mode configuration parameter: " + String(PARAMETER_NAME_FADE_LENGTH));
        } else {
            parameters.fadeLength = (uint8_t) atoi(request->getParam(PARAMETER_NAME_FADE_LENGTH, true)->value().c_str());
        }
    }

    strip.configureMode(mode, parameters);

    if (request->hasParam("start_mode", true)) {
        Command command;
        command.command = COMMAND_SET_MODE;
        command.parameter1 = mode;
        commandQueue.pushCommand(command);
    }
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Starts the firmware update process and
            installs the specified version on the controller.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void updateFirmware(AsyncWebServerRequest *request) {
    const char* parameters[] = {"version"};
    
    if (!checkPostParameters(request, parameters, 1)) {
        return;
    }

    String resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "text/javascript", resultString);
    
    String version = request->getParam("version", true)->value();
    updateManager.updateSystem(version);
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Resets the network configuration and restarts
            this controller.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void resetNetworkConfigurationRequest(AsyncWebServerRequest *request) {
    String resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "text/javascript", resultString);

    resetNetworkConfiguration();
}
#pragma endregion

#pragma region Routes
/******************************************************************************/
/*!
  @brief    Routes for receiving data and commands by HTTP requests.
*/
/******************************************************************************/
void createCommandRoutes() {
    /* If in network configuration mode, load only configuration command and page */
    if (bootMode == BOOT_MODE_NETWORK_CONFIGURATION) {
        server.on("/", ASYNC_HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/templates/network_config.html", String());
        });

        /* CSS files */
        server.serveStatic("/colors.css", SPIFFS, "/css/colors.css");
        server.serveStatic("/base_web_styles.css", SPIFFS, "/css/base_web_styles.css");
        server.serveStatic("/styles_mobile.css", SPIFFS, "/css/styles_mobile.css");

        /* JavaScript files */
        server.serveStatic("/jquery.min.js", SPIFFS, "/js/jquery.min.js");
        server.serveStatic("/base_utilities.js", SPIFFS, "/js/base_utilities.js");
        server.serveStatic("/network_config_page.js", SPIFFS, "/js/network_config_page.js");
        
        /* Images */
        server.serveStatic("/favicon.ico", SPIFFS, "/images/favicon.ico");

        server.on(CMD_CONFIGURE_NETWORK, ASYNC_HTTP_POST, configureNetwork);
        return;
    }

    server.on(CMD_GET_HOSTNAME, ASYNC_HTTP_GET, [](AsyncWebServerRequest *request) {
        String resultString = "{\"hostname\":\"" + networkConfig.hostname + "\"}";
        request->send(HTTP_CODE_OK, "text/javascript", resultString);
    });
    
    server.on(CMD_GET_STATES, ASYNC_HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTP_CODE_OK, "text/javascript", generateStatesJSON());
    });

    server.on(CMD_GET_MODE_CONFIGURATIONS, ASYNC_HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTP_CODE_OK, "text/javascript", generateModeConfigurationJSON());
    });

    server.on(CMD_GET_CONFIGURATION, ASYNC_HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTP_CODE_OK, "text/javascript", generateConfigurationJSON());
    });

    server.on(CMD_GET_LEDS, ASYNC_HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(HTTP_CODE_OK, "text/javascript", strip.getPixels());
    });
    
    server.on(CMD_GET_LOGS, ASYNC_HTTP_GET, downloadLogs);
    server.on(CMD_SET_CONFIGURATION, ASYNC_HTTP_POST, setConfiguration);
    server.on(CMD_SET_POWER, ASYNC_HTTP_POST, setPower);
    server.on(CMD_DRAW_LEDS, ASYNC_HTTP_POST, drawPixels);
    server.on(CMD_SET_BRIGHTNESS, ASYNC_HTTP_POST, setBrightness);
    server.on(CMD_SET_MODE, ASYNC_HTTP_POST, setMode);
    server.on(CMD_CONFIGURE_MODE, ASYNC_HTTP_POST, configureMode);
    server.on(CMD_UPDATE_FIRMWARE, ASYNC_HTTP_POST, updateFirmware);

    server.on(CMD_REBOOT, ASYNC_HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(HTTP_CODE_OK);
        rebootDelay.once(1, rebootTicker);                                      //Reboot delay and return for HTTP to return response
    });
}
#pragma endregion

#pragma region General Utilities
/******************************************************************************/
/*!
  @brief    Checks whether local door state has changed.
*/
/******************************************************************************/
void checkSensor() {
    bool pinState1 = !digitalRead(LOCAL_SENSOR1_PIN);
    bool pinState2 = !digitalRead(LOCAL_SENSOR2_PIN);

    if (sensorInverted) {
        pinState1 = !pinState1;
        pinState2 = !pinState2;
    }

    /* Don't take action when is the same */
    if (localDoorState == (pinState1 | pinState2)) {
        return;
    }

    localDoorState = pinState1 | pinState2;
    
    if (localDoorState) {
        l.logi("Local door opened");
    } else {
        l.logi("Local door closed");
    }

    Command command;
    command.command = COMMAND_DOOR_CHANGE;
    command.parameter1 = (uint8_t) localDoorState;

    commandQueue.pushCommand(command);
    vTaskDelay(DEBOUNCE_TIME);
    
    /* Send to master controller */
    HTTPClient http;
    String url = String(MASTER_SERVER_ADDRESS) + MASTER_CMD_SET_SENSOR_STATE;
    
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT);
    
    if (http.POST("state=" + String((uint8_t) localDoorState)) == HTTP_CODE_OK) {
        l.logi("Sended doorsensor state to masterController");
    } else {
        l.logi("Couldnot send state to masterController");
    }
    http.end();
}

/******************************************************************************/
/*!
  @brief    Checks whether local door state has changed.
*/
/******************************************************************************/
CRGB hexStringToRGB(String hexColorString) {
    uint32_t redMask = 0xFF0000;
    uint32_t greenMask = 0xFF00;
    uint32_t blueMask = 0xFF;
    
    uint32_t hexColor = 0;
    
    sscanf(hexColorString.c_str(), "%x", &hexColor);

    uint8_t r = (hexColor & redMask) >> 16;
    uint8_t g = (hexColor & greenMask) >> 8;
    uint8_t b = (hexColor & blueMask);
    
    return CRGB(r, g, b);
}

/******************************************************************************/
/*!
  @brief    Generates a JSON string containing HTTP response information.
  @param    endpoint            Endpoint that was called
  @param    responseCode        HTTP response code
  @param    message             Additional message
  @returns  String              JSON string
*/
/******************************************************************************/
String generateResponseJson(String endpoint, uint16_t responseCode, String message) {
    String responseString = "{\"endpoint\": ";
    responseString += "\"" + endpoint + "\", ";
    responseString += "\"statusCode\": ";
    responseString += String(responseCode) + ", ";
    responseString += "\"resultMessage\": ";
    responseString += "\"" + message + "\"}";
    
    return responseString;
}

/******************************************************************************/
/*!
  @brief    Replaces placeholders in front-end with data from back-end.
*/
/******************************************************************************/
String generateStatesJSON() {
    String power = "\"power\":" + (String) strip.getPower();
    String sdMounted = "\"sd_card_inserted\":" + (String) memoryManager.getSdMounted();
    String brightness = "\"brightness\" : " + (String) strip.getBrightness();
    String mode = "\"mode\":" + (String) strip.getMode();
    String sensorState = "\"sensor_state\":" + String(localDoorState);

    String jsonString = "{" + power;
    jsonString += ", " + sdMounted;
    jsonString += ", " + brightness;
    jsonString += ", " + mode;
    jsonString += ", " + sensorState + "}";

    return jsonString;
}

/******************************************************************************/
/*!
  @brief    Replaces placeholders in front-end with data from back-end.
*/
/******************************************************************************/
String generateConfigurationJSON() {
    String idString = "\"id\":" + String(id);
    String hostname = "\"hostname\":\"" + networkConfig.hostname + "\"";
    String numberOfLeds = "\"number_of_leds\":" + (String) strip.getNumberOfLeds();
    String driver = "\"model_id\":" + String(strip.getDriver());
    String powerAnimation = "\"power_animation\":" + String(strip.getPowerAnimation());
    String firmwareVersion = "\"firmware_version\":\"" + String(FIRMWARE_VERSION) + "\"";
    String sensorEnabledStr = "\"has_sensor\":" + String(sensorEnabled);
    String sensorInvertedStr = "\"sensor_inverted\":" + String(sensorInverted);
    String sensorModelStr = "\"sensor_model\":" + String(sensorModel);

    String jsonString = "{" + idString;
    jsonString += ", " + hostname;
    jsonString += ", " + driver;
    jsonString += ", " + numberOfLeds;
    jsonString += ", " + powerAnimation;
    jsonString += ", " + firmwareVersion;
    jsonString += ", " + sensorEnabledStr;
    jsonString += ", " + sensorInvertedStr;
    jsonString += ", " + sensorModelStr + "}";

    l.logd(jsonString);
    return jsonString;
}

/******************************************************************************/
/*!
  @brief    Replaces placeholders in front-end with data from back-end.
*/
/******************************************************************************/
String generateModeConfigurationJSON() {
    String jsonString = "[";

    for (uint8_t mode = 1; mode < NUM_MODES; mode++) {
        String modeString = memoryManager.getModeJsonString(mode);
        jsonString += modeString;

        if (mode < NUM_MODES-1) {
            jsonString += ",";
        }
    }
    jsonString += "]";
    
    return jsonString;
}

/******************************************************************************/
/*!
  @brief    Sends a connection request to the master server.
*/
/******************************************************************************/
void connectToMasterServer() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    
    HTTPClient http;
    String url = String(MASTER_SERVER_ADDRESS) + MASTER_CMD_REQUEST_CONNECTION;
    String payload = String("hostname=") + WiFi.getHostname();
    
    http.begin(url);
    http.setTimeout(10000);
    int16_t responseCode = http.POST(payload);
    http.end();                                                                 //Free resources

    while (id == DEFAULT_ID && responseCode != HTTP_CODE_OK) {
        vTaskDelay(10000);

        http.begin(url);
        http.setConnectTimeout(HTTP_TIMEOUT);
        responseCode = http.POST(payload);
        http.end();

        l.logi("Connecting to master controller", false);
        strip.setMode(SYSTEM_MODE_PULSES);
    }
    
    if (responseCode == HTTP_CODE_OK) {
        l.logi("Connected to master controller");
    } else {
        l.logw("Could not connect to master controller");
    }
    http.end();
}

/******************************************************************************/
/*!
  @brief    Checks whether the required parameters of an HTTP request are
            present.
  @param    request             Pointer to the HTTP request
  @param    parameters          Array of parameter names
  @param    numberOfParameters  Number of parameters to check
  @param    needsAll            If true, all the parameters are required
  @param    post                If true, it is an HTTP POST request
  @returns  bool                True if parameters are present
*/
/******************************************************************************/
bool checkParameters(AsyncWebServerRequest *request, const char* neededParameters[], uint8_t numberOfParameters, bool needsAll, bool post) {
    String resultString;

    if (needsAll) {
        if (numberOfParameters == 0) {
            resultString = generateResponseJson(request->url(), HTTP_CODE_BAD_REQUEST, "Missing parameters");
            request->send(HTTP_CODE_BAD_REQUEST, "application/json", resultString);
            return false;
        }

        for (uint8_t i = 0; i < numberOfParameters; i++) {
            if (!request->hasParam(neededParameters[i], post)) {
                resultString = generateResponseJson(request->url(), HTTP_CODE_BAD_REQUEST, "Missing parameters");
                request->send(HTTP_CODE_BAD_REQUEST, "application/json", resultString);
                return false;
            }
        }

        return true;
    }

    for (uint8_t i = 0; i < numberOfParameters; i++) {
        if (request->hasParam(neededParameters[i], post)) {
            return true;
        }
    }

    resultString = generateResponseJson(request->url(), HTTP_CODE_BAD_REQUEST, "Missing parameters");
    request->send(HTTP_CODE_BAD_REQUEST, "application/json", resultString);
    return false;
}

/******************************************************************************/
/*!
  @brief    Checks whether the required parameters of an HTTP POST request are
            present.
  @param    request             Pointer to the HTTP request
  @param    parameters          Array of parameter names
  @param    numberOfParameters  Number of parameters to check
  @param    needsAll            If true, all the parameters are required
  @returns  bool                True if parameters are present
*/
/******************************************************************************/
bool checkPostParameters(AsyncWebServerRequest *request, const char* neededParameters[], uint8_t numberOfParameters, bool needsAll) {
    return checkParameters(request, neededParameters, numberOfParameters, needsAll, true);
}

/******************************************************************************/
/*!
  @brief    Executes the pending commands.
  @param    parameters          Pointer to the parameters of the task
*/
/******************************************************************************/
void executeCommands(void *parameters) {
    while (true) {
        if (commandQueue.isEmpty()) {
            vTaskDelay(10);
            continue;
        }

        Command command = commandQueue.getCommand();

        switch (command.command) {
            case COMMAND_SET_POWER:
                strip.setPower((bool) command.parameter1);
                commandQueue.popCommand();
                while (strip.getState() != _READY_TO_RUN && strip.getState() != _LOOPING) continue;
                break;
            case COMMAND_SET_BRIGHTNESS:
                strip.setBrightness((uint8_t) command.parameter1);
                commandQueue.popCommand();
                while (strip.getState() != _READY_TO_RUN && strip.getState() != _LOOPING) continue;
                break;
            case COMMAND_SET_MODE:
                strip.setMode((uint8_t) command.parameter1);
                commandQueue.popCommand();
                while (strip.getState() != _READY_TO_RUN && strip.getState() != _LOOPING) continue;
                break;
            case COMMAND_DOOR_CHANGE:
                strip.doorHandler((bool) command.parameter1);
                commandQueue.popCommand();
                while (strip.getState() != _READY_TO_RUN && strip.getState() != _LOOPING && strip.getState() != _WAIT_FOR_DOOR_CLOSED) continue;
                break;
            
            default:
                l.logw("Command not found, deleting it");
                commandQueue.popCommand();
                break;
        }
    }
}

/******************************************************************************/
/*!
  @brief    Reboots, in function because of Ticker class.
*/
/******************************************************************************/
void rebootTicker() {
    ESP.restart();
}

/******************************************************************************/
/*!
  @brief    Counts the number of logs in the current log file. Needs to be
            called on setup.
*/
/******************************************************************************/
void countNumberOfLogs() {
    _numberOfLogs = 0;
    File logsFile = SPIFFS.open(memoryManager.joinPaths(LOGS_DIRECTORY, LOGS_FILE), FILE_READ);

    while (logsFile.available()) {
        logsFile.readStringUntil('\n');
        _numberOfLogs++;
    }

    logsFile.close();
}

/******************************************************************************/
/*!
  @brief    Logs the reason for the last reset.
*/
/******************************************************************************/
void logResetReason() {
    esp_reset_reason_t resetReason = esp_reset_reason();

    switch (resetReason) {
        case ESP_RST_UNKNOWN:
            l.logw("Last reset reason: Can not be determined");
            break;
        case ESP_RST_POWERON:
            l.logd("Last reset reason: Power-on event");
            break;
        case ESP_RST_EXT:
            l.logw("Last reset reason: By external pin (not applicable for ESP32)");
            break;
        case ESP_RST_SW:
            l.logd("Last reset reason: Software reboot");
            break;
        case ESP_RST_PANIC:
            l.logw("Last reset reason: Software reset due to exception/panic");
            break;
        case ESP_RST_INT_WDT:
            l.logw("Last reset reason: Interrupt watchdog (software or hardware)");
            break;
        case ESP_RST_TASK_WDT:
            l.logw("Last reset reason: Task watchdog");
            break;
        case ESP_RST_WDT:
            l.logw("Last reset reason: Other watchdogs than task and interrupt");
            break;
        case ESP_RST_DEEPSLEEP:
            l.logi("Last reset reason: Exiting deep sleep mode");
            break;
        case ESP_RST_BROWNOUT:
            l.logw("Last reset reason: Brownout reset (software or hardware)");
            break;
        case ESP_RST_SDIO:
            l.logw("Last reset reason: Reset over SDIO");
            break;
        default:
            l.logw("Last reset reason: Unknown reset reason " + String(resetReason));
    }
}
#pragma endregion

#pragma region File Utilities
/******************************************************************************/
/*!
  @brief    Loads the configuration from non-volatile memory into the RAM.
*/
/******************************************************************************/
void loadNetworkConfiguration() {
    nvMemory.begin(NV_MEM_CONFIG);

    /* When SSID is undefined, network is not configured yet */
    String ssid = nvMemory.getString("ssid");
    if (ssid == "") {
        nvMemory.end();
        return;
    }

    /* When SSID is defined, network is configured. Load from NV memory. */
    networkConfig.ssid = securityManager.decrypt(ssid.c_str());
    networkConfig.password = securityManager.decrypt(nvMemory.getString("password").c_str());
    networkConfig.hostname = nvMemory.getString("hostname");
    nvMemory.end();

    l.logi("Network configuration loaded from NV memory", false);
}
#pragma endregion

#pragma region WiFi and network configuration
/******************************************************************************/
/*!
  @brief    Initializes WiFi. When a network configuration is found, it
            connects to the network. Otherwise, it will start as access point
            to configure the network.
*/
/******************************************************************************/
void initializeWifi() {
    l.logi("Initializing WiFi", false);

    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    String uniqueApId = String(baseMac[0], HEX) + String(baseMac[2], HEX) + String(baseMac[4], HEX);
    String configNetworkApSsid = NETWORK_CONFIG_AP_SSID + uniqueApId;

    /* If network is not initialized, start as access point */
    if (networkConfig.ssid == "") {
        l.logw("No configured network found. Starting as AP and waiting for network configuration");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(configNetworkApSsid, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONNECTIONS);
        bootMode = BOOT_MODE_NETWORK_CONFIGURATION;
        return;
    }

    if (networkConfig.hostname != "") {
        WiFi.setHostname(networkConfig.hostname.c_str());
    } else {
        String tempHostname = String("zyrax_") + uniqueApId;
        WiFi.setHostname(tempHostname.c_str());
        l.logw("Hostname not configured");
    }
    
    connectToWiFi();

    /* Save dynamic IP configuration */
    if (networkConfig.ip != WiFi.localIP()) {
        networkConfig.ip = WiFi.localIP();

        nvMemory.begin(NV_MEM_CONFIG);
        nvMemory.putString("ip", networkConfig.ip.toString());
        nvMemory.end();
    }
    if (networkConfig.gateway != WiFi.gatewayIP()) {
        networkConfig.gateway = WiFi.gatewayIP();

        nvMemory.begin(NV_MEM_CONFIG);
        nvMemory.putString("gateway", networkConfig.gateway.toString());
        nvMemory.end();
    }
    if (networkConfig.subnet != WiFi.subnetMask()) {
        networkConfig.subnet = WiFi.subnetMask();

        nvMemory.begin(NV_MEM_CONFIG);
        nvMemory.putString("subnet", networkConfig.subnet.toString());
        nvMemory.end();
    }
    if (networkConfig.dns != WiFi.dnsIP(0)) {
        networkConfig.dns = WiFi.dnsIP(0);

        nvMemory.begin(NV_MEM_CONFIG);
        nvMemory.putString("dns", networkConfig.dns.toString());
        nvMemory.end();
    }
}

/******************************************************************************/
/*!
  @brief    Connects to the configured Wi-Fi network.
*/
/******************************************************************************/
void connectToWiFi() {
    WiFi.begin(networkConfig.ssid, networkConfig.password);

    /* Get if there is a connection timeout, else try infinitely */
    nvMemory.begin(NV_MEM_CONFIG, true);
    bool tryConnect = nvMemory.getBool("tryConnect", true);
    nvMemory.end();

    /*
     * Only try one time and reset flag. When the connection is established,
     * keep configuration. Otherwise, the configuration will be resetted.
     */
    if (tryConnect) {
        l.logi("Trying to connect for a minute");
    }

    uint8_t retry = 0;

    /* Try to connect */
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(1000);
        Serial.print(".");
        
        if (retry >= WIFI_TRY_CONNECT_TIMEOUT) {
            Serial.println("");

            if (tryConnect) {
                break;
            } else {
                l.logw("Tried to connect to network [" + networkConfig.ssid + "], did not work. Restarting");
                ESP.restart();
            }
        }
        retry++;
    }

    /* When only trying to connect and still disconnected, reset configuration */
    if (tryConnect && retry >= WIFI_TRY_CONNECT_TIMEOUT) {
        l.loge("Tried to connect to network [" + networkConfig.ssid + "], did not work");
        resetNetworkConfiguration();
    }
    
    l.logi("WiFi strength [" + String(WiFi.RSSI()) + String("dBm]"), false);
    //l.logd("IP: " + WiFi.localIP().toString());

    /* Reset try connect flag, network is verified */
    if (tryConnect) {
        nvMemory.begin(NV_MEM_CONFIG);
        nvMemory.putBool("tryConnect", false);
        nvMemory.end();
    }
    
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
}

/******************************************************************************/
/*!
  @brief    Resets the network configuration and restarts this controller.
*/
/******************************************************************************/
void resetNetworkConfiguration() {
    nvMemory.begin(NV_MEM_CONFIG);
    nvMemory.putString("ssid", "");
    nvMemory.putString("password", "");
    nvMemory.putString("hostname", "");
    nvMemory.putBool("tryConnect", true);
    nvMemory.putString("ip", DEFAULT_IP);
    nvMemory.putString("gateway", DEFAULT_IP);
    nvMemory.putString("subnet", DEFAULT_IP);
    nvMemory.end();
    
    l.logw("Resetted network configuration, rebooting now");
    ESP.restart();
}

/******************************************************************************/
/*!
  @brief    Handles HTTP request. Sets the network configuration with the
            specified configuration and restarts the controller.
  @param    request             Pointer to the HTTP request
*/
/******************************************************************************/
void configureNetwork(AsyncWebServerRequest *request) {
    const char* parameters[] = {"ssid",
                                "password",
                                "hostname"};

    if (!checkPostParameters(request, parameters, 3)) {
        return;
    }

    String resultString = generateResponseJson(request->url(), HTTP_CODE_OK);
    request->send(HTTP_CODE_OK, "application/json", resultString);
    
    networkConfig.ssid = request->getParam("ssid", true)->value().c_str();
    networkConfig.password = request->getParam("password", true)->value().c_str();
    networkConfig.hostname = request->getParam("hostname", true)->value().c_str();

    /* Save in non-volatile memory */
    nvMemory.begin(NV_MEM_CONFIG);
    nvMemory.putString("ssid", securityManager.encrypt(networkConfig.ssid.c_str()));
    nvMemory.putString("password", securityManager.encrypt(networkConfig.password.c_str()));
    nvMemory.putString("hostname", networkConfig.hostname);
    nvMemory.putBool("tryConnect", true);
    nvMemory.end();

    l.logi("Controller needs to be rebooted in order to finish configuration");
    rebootDelay.once(1, rebootTicker);                                          //Reboot delay and return for HTTP to return response
}
#pragma endregion