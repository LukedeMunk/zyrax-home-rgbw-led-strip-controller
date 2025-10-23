/******************************************************************************/
/*
 * File:    Globals.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Global datatypes, variables and constants.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef GLOBALS_H
#define GLOBALS_H
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "stdint.h"                                                             //For size defined int types
#include "WiFi.h"                                                               //For IPAddress type
#include "FastLED.h"                                                            //For CRGB color class


#pragma region Pins
/* SD - ESP32 S3 pins (Don't change) */
#define SD_MMC_CLK_PIN                  39
#define SD_MMC_DATA_PIN                 40  //ATTENTION TODO
#define SD_MMC_CMD_PIN                  38

/* Ledstrip */
#define LEDSTRIP_DATA_PIN               MOSI
#define LEDSTRIP_CLOCK_PIN              SCK
#pragma endregion

#pragma region Boot modes
#define BOOT_MODE_BOOTING               0
#define BOOT_MODE_NORMAL                1
#define BOOT_MODE_ERROR                 2
#define BOOT_MODE_NETWORK_CONFIGURATION 3
#define BOOT_MODE_FATAL_ERROR           4
#pragma endregion

#pragma region OTA states
#define OTA_STATE_IDLE                  0
#define OTA_STATE_DOWNLOADING_FIRMWARE  1
#define OTA_STATE_INSTALLING_FIRMWARE   3
#define OTA_STATE_FAILED                4
#define OTA_STATE_FINISHED              5
#define OTA_STATE_WAITING               6
#pragma endregion

#pragma region Endpoints
/* Getters */
#define CMD_GET_OTA_STATE               "/get_ota_state"
#define CMD_GET_HOSTNAME                "/get_hostname"
#define CMD_GET_STATES                  "/get_states"
#define CMD_GET_MODE_CONFIGURATIONS     "/get_mode_configurations"

/* Setters */
#define CMD_SET_POWER                   "/set_power"
#define CMD_SET_BRIGHTNESS              "/set_brightness"
#define CMD_SET_MODE                    "/set_mode"
#define CMD_SET_CONFIGURATION           "/set_configuration"

/* Utility commands */
#define CMD_SET_INSTALLATION_MODE       "/set_installation_mode"
#define CMD_REBOOT  	        	    "/reboot"
#define CMD_RESET_SD                    "/reset_sd"
#define CMD_RESET_NETWORK_CONFIGURATION "/reset_network_configuration"
#define CMD_CONFIGURE_MODE              "/configure_mode"
#define CMD_REBOOT                      "/reboot"
#define CMD_GET_LOGS                    "/download_logs"
#define CMD_DELETE_LOGS                 "/delete_logs"
#define CMD_UPDATE_FIRMWARE             "/update_firmware"
#define CMD_GET_CONFIGURATION           "/get_configuration"
#define CMD_GET_LEDS                  "/get_leds"
#define CMD_DRAW_LEDS                 "/draw_leds"
#define CMD_CONFIGURE_NETWORK           "/configure_network"


/* Endpoint commands of master controller */
#define MASTER_CMD_SET_OTA_STATE        "/set_ota_state"
#define MASTER_CMD_DOWNLOAD_OTA_FILE    "/get_ota_file"
#define MASTER_CMD_REQUEST_CONNECTION   "/ledstrip_request_connection"
#define MASTER_CMD_SET_SENSOR_STATE            "/set_ledstrip_sensor_state"
#pragma endregion

#pragma region Filesystem
/* Directories */
#define SYSTEM_DIRECTORY                "/system"
#define LOGS_DIRECTORY                  "/system/logs"
#define OTA_FIRMWARE_DIRECTORY          "/system/ota"

/* Files */
#define LOGS_FILE                       "logs.log"
#pragma endregion

#pragma region Defaults
#define DEFAULT_VERSION                 "v0.0.0"
#define DEFAULT_IP                      "0.0.0.0"
#define DEFAULT_ID                      255
#pragma endregion

#pragma region Non-volatile memory namespaces (limited to 15 chars)
#define NV_MEM_CONFIG                   "config"
#pragma endregion

#pragma region Others
#define MAX_BRIGHTNESS                  255                                     //Duty cycle is same as brightness

#define BYTES_IN_KBYTE                  1024
#define FILE_CHUNK_SIZE                 32*BYTES_IN_KBYTE                       //32kB buffer

#define HTTP_TIMEOUT                    1000
#define HTTP_RETRIES                    3

#define MAX_NUMBER_OF_LOGS              1000

#define DOWNLOAD_RETRIES                10
#define SD_MOUNTPOINT                   "/sd"

/* Drivers */
#define _WS2801                         0
#define _WS2812B                        1
#define _SK6812                         2

/* Modes */
#define MODE_COLOR                      1                                       //Modes on master start with 1
#define MODE_FADE                       2
#define MODE_GRADIENT                   3
#define MODE_BLINK                      4
#define MODE_SCAN                       5
#define MODE_THEATER                    6
#define MODE_SINE                       7
#define MODE_BOUNCING_BALLS             8
#define MODE_DISSOLVE                   9
#define MODE_SPARKLE                    10
#define MODE_FIREWORKS                  11
#define MODE_FIRE                       12
#define MODE_SWEEP                      13
#define MODE_COLOR_TWINKELS             14
#define MODE_METEOR_RAIN                15
#define MODE_COLOR_WAVES                16
#define MODE_TEMPLATE_1                 17
#define MODE_TEMPLATE_2                 18
#define MODE_TEMPLATE_3                 19
#define MODE_TEMPLATE_4                 20
#define MODE_TEMPLATE_5                 21
#define MODE_TEMPLATE_6                 22
#define MODE_TEMPLATE_7                 23
#define MODE_TEMPLATE_8                 24
#define MODE_TEMPLATE_9                 25
#define MODE_TEMPLATE_10                26
//#define SUNRISE                       12
//#define SUNSET                        13
#define MODE_DRAWING                    50

/* System modes */
#define SYSTEM_MODE_PULSES              100
#define SYSTEM_MODE_ALARM               101

#define NUM_MODES                       17                                      //Just add up num of modes + 1 because ID starts with 1 on master controller MINUS SYSTEM modes

#define _POWER_FADE                     0
#define _POWER_DISSOLVE                 1
#define _POWER_SWEEP                    2
#define _POWER_DUAL_SWEEP               3
#define _POWER_MULTI_SWEEP              4

#define NUM_POWER_ANIMATIONS            5

/* States */
#define _FADE_BRIGHTNESS                5
#define _FADE_TO_SINGLE_COLOR           6
#define _FADE_TO_MULTIPLE_COLOR         7

#define _READY_TO_RUN                   8
#define _LOOPING                        9
#define _WAIT_FOR_DOOR_CLOSED           10


#define PARAMETER_NAME_MIN_COLOR_POS    "min_color_pos"
#define PARAMETER_NAME_MAX_COLOR_POS    "max_color_pos"
#define PARAMETER_NAME_COLOR1           "color1"
#define PARAMETER_NAME_COLOR2           "color2"
#define PARAMETER_NAME_USE_GRADIENT1    "use_gradient1"
#define PARAMETER_NAME_USE_GRADIENT2    "use_gradient2"
#define PARAMETER_NAME_SEGMENT_SIZE     "segment_size"
#define PARAMETER_NAME_TAIL_LENGTH      "tail_length"
#define PARAMETER_NAME_WAVE_LENGTH      "wave_length"
#define PARAMETER_NAME_TIME_FADE        "time_fade"                 //Todo change to fade delay
#define PARAMETER_NAME_DELAY            "delay"
#define PARAMETER_NAME_DELAY_BETWEEN    "delay_between"
#define PARAMETER_NAME_RANDOMNESS_DELAY "randomness_delay"
#define PARAMETER_NAME_INTENSITY        "intensity"
#define PARAMETER_NAME_DIRECTION        "direction"
#define PARAMETER_NAME_NUMBER_OF_ELEMENTS   "number_of_elements"
#define PARAMETER_NAME_PALETTE              "palette"
#define PARAMETER_NAME_FADE_LENGTH          "fade_length"           //Todo change to fade delay

#define NUMBER_OF_MODE_PARAMETERS       18
#define MAX_SEGMENT_SIZE                21

#define PALETTE_RANDOM                  0
#define PALETTE_YELLOW_RED              1
#define PALETTE_PURPLE_BLUE             2
#define PALETTE_GREEN_BLUE              3
#define PALETTE_BLUE_GREEN              4
#define PALETTE_CLOUD_COLORS            10
#define PALETTE_LAVA_COLORS             11
#define PALETTE_OCEAN_COLORS            12
#define PALETTE_FOREST_COLORS           13

#define DIRECTION_LEFT                  0
#define DIRECTION_RIGHT                 1

#define SENSOR_MODEL_CONTACT_SWITCH     0
#pragma endregion

#pragma region Structures
struct NetworkConfiguration {
    String ssid = "";
    String password;
    String hostname;
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;
};

struct ModeParameters {
    uint8_t minColorPos = 0;
    uint8_t maxColorPos = 255;
    CRGB color1 = CRGB(255, 255, 255);
    CRGB color2 = CRGB(0, 0, 0);
    bool useGradient1 = false;
    bool useGradient2 = false;
    uint8_t segmentSize = 2;
    uint8_t tailLength = 0;
    uint8_t waveLength = 10;
    uint16_t timeFade = 100;
    uint16_t delay = 500;
    uint16_t delayBetween = 100;
    uint8_t randomnessDelay = 0;
    uint8_t intensity = 0;
    uint8_t direction = DIRECTION_LEFT;
    uint8_t numberOfElements = 3;
    uint8_t palette = PALETTE_YELLOW_RED;
    uint8_t fadeLength = 0;
    uint8_t colorPosition = 0;                                                  //state
};
#define MAX_FADE_TIME 100
#define MAX_WAVE_LENGTH 10
#pragma endregion

#pragma region Variables
inline uint16_t _numberOfLogs;
#pragma endregion
#endif