/******************************************************************************/
/*
 * File:    MemoryManager.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   MemoryManager class for handling most of the SD card filesystem
 *          interaction. Main tasks are reading files.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef SD_MANAGER_H
#define SD_MANAGER_H
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "stdint.h"                                                             //For size defined int types
#include "Preferences.h"                                                        //For non-volatile memory functionality
#include "FS.h"
#include "SD_MMC.h"                                                             //For file interaction with Micro SD card
#include "SPIFFS.h"                                                             //For file interaction with internal SPI Flash File System

#include "Logger.h"                                                             //For printing and saving logs
#include "Configuration.h"                                                      //For configuration variables and global constants

class MemoryManager {
    public:
        MemoryManager();
        bool initialize();
        ModeParameters loadModeParameters(uint8_t mode);
        String getModeJsonString(uint8_t mode);
        void writeModeParameters(uint8_t mode, ModeParameters parameters);
        bool createFolderIfNotExists(String path);
        bool getSdMounted();

        /* Reset functionality */
        void resetNVvariables();
        
        /* Utilities */
        bool copyFile(String filePath, String copyPath);
        String joinPaths(String path1, String path2, String path3="");
        uint32_t rgbToHex(CRGB color);
        bool modeHasParameter(uint8_t mode, String parameterName);
        uint8_t getnumberOfModeParameters(uint8_t mode);
        
    private:
        String _generateModeParameterJsonString(String parameterName, String parameterValue, bool isString=false);
        bool _checkFileSystemStructure();
        bool _checkDirectory(String directory);
        bool _checkFile(String filePath, String defaultFilePath = "");
        CRGB _stringToCRGB(String colorString);
        String _crgbToString(CRGB color);

        Logger _l;
        Preferences _nvMemory;
        bool _sdMounted;
};
#endif