/******************************************************************************/
/*
 * File:    UpdateManager.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   UpdateManager class for (O)ver-(T)he-(A)ir updates. Handles
 *          firmware file downloads, installation and progression states.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "stdint.h"                                                             //For size defined int types
#include "Preferences.h"                                                        //For non-volatile memory functionality
#include "HTTPClient.h"                                                         //For sending OTA states to the master controller
#include "Update.h"                                                             //For starting firmware updates on this chip
#include "SPIFFS.h"                                                             //For checking SPIFFS version
#include "WiFiClient.h"                                                         //For downloading the firmware files
#include "SD_MMC.h"                                                             //For file interaction with Micro SD card

#include "MemoryManager.h"                                                      //For managing files on the SD card
#include "Logger.h"                                                             //For printing and saving logs
#include "Configuration.h"                                                      //For configuration variables and global constants
#include "Version.h"                                                            //For easy version conversion and comparison

#define MAX_NUM_POWER_CYCLES            5

#pragma region OTA states
#define OTA_STATE_IDLE                  0
#define OTA_STATE_DOWNLOADING_FIRMWARE  1
#define OTA_STATE_INSTALLING_FIRMWARE   2
#define OTA_STATE_FINISHED              3
#define OTA_STATE_WAITING               4
#pragma endregion

#define DEFAULT_VERSION                 "0_0_0"

class UpdateManager {
    public:
        UpdateManager();
        void initialize(uint8_t id);
        void updateSystem(Version version);

        /* Getters */
        uint8_t getState();

    private:
        void _sendState();

        static void _dispatchUpdateSystem(void* parameter);
        void _taskUpdateSystem();
        bool _downloadOtaFile();
        void _installOtaFile();

        TaskHandle_t _taskHandlerUpdateSystem = NULL;

        Logger _l;
        Preferences _nvMemory;
        MemoryManager _memoryManager;
        uint8_t _state;
        Version _updateToVersion;
        uint8_t _ledstripId;

        uint8_t _powerCycles;
};
#endif