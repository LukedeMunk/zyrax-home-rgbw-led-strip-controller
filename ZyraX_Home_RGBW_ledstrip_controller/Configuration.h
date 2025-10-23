/******************************************************************************/
/*
 * File:    Configuration.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Global configuration variables and constants.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H
#include "Globals.h"                                                            //Global variables and constants

#define FIRMWARE_VERSION                "v0.9.0"

#define MASTER_SERVER_ADDRESS           "https://mastercontroller.local"
//#define MASTER_SERVER_ADDRESS           "http://192.168.2.37:5000"              //TODO_IN_PRODUCTION: Remove

#define DEBOUNCE_TIME                   1000


/* Animation delays */
#define COLOR_DELAY                     3                                       //Delay between frames, in ms
#define BRIGHTNESS_DELAY                5                                       //Delay between frames, in ms

/* Local sensor pins */
#define LOCAL_SENSOR1_PIN               5
#define LOCAL_SENSOR2_PIN               6

/* Others */
#define MAX_BRIGHTNESS                  180
#define MAX_NUMBER_LEDS                 250


/* Network credentials */
#define NETWORK_CONFIG_AP_SSID          "Config network ESP32_"
#define AP_PASSWORD                     "NetworkConfig!"
#define CONNECTION_TIMEOUT              500
#define WIFI_POLL_TIME                  30                                      //Interval of WiFi checks (s)
#define WIFI_TRY_CONNECT_TIMEOUT        20                                      //For when the controller tries to connect to a new configured network (s)
#define COMMAND_SERVER_PORT             80                                      //Cannot be port 81, since that port is used for audio streaming

#define AP_CHANNEL                      1
#define AP_MAX_CONNECTIONS              1
#endif