/******************************************************************************/
/*
 * File:    Logger.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Logger class for printing (Serial) and saving logs.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef LOGGER_H
#define LOGGER_H
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "stdint.h"                                                             //For size defined int types
#include "SD_MMC.h"                                                             //For file interaction with Micro SD card

#include "Configuration.h"                                                      //For configuration variables and global constants

/* Log types */
#define LOG_TYPE_DEBUG                  0
#define LOG_TYPE_INFO                   1
#define LOG_TYPE_WARNING                2
#define LOG_TYPE_ERROR                  3
#define LOG_TYPE_FATAL_ERROR            4
#define LOG_TYPE_NONE                   100

#define MAX_NUMBER_OF_LOGS              500

class Logger {
    public:
        Logger(const char* tag="", uint8_t logLevel=LOG_TYPE_DEBUG);
        void setTag(const char* tag);
        void setLogLevel(uint8_t logLevel);

        /* Main functionality */
        void logd(String logString);
        void logd(const char* logString);

        void logi(String logString, bool saveToFile=true);
        void logi(const char* logString, bool saveToFile=true);

        void logw(String logString, bool saveToFile=true);
        void logw(const char* logString, bool saveToFile=true);

        void loge(String logString, bool saveToFile=true);
        void loge(const char* logString, bool saveToFile=true);

        void logfe(String logString, bool saveToFile=true);
        void logfe(const char* logString, bool saveToFile=true);

        /* Utilities */
        String generateJsonLog(uint8_t type, const char* log);
        void markLogsAsRead();

    private:
        void _log(const char* logString, uint8_t type, bool saveToFile=true);
        String _joinPaths(String path1, String path2, String path3="");

        const char* _tag;
        uint8_t _logLevel;
};
#endif