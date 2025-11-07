/******************************************************************************/
/*
 * File:    Logger.cpp
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Logger class for printing (Serial) and saving logs.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#include "Logger.h"

#pragma region Main class functionality
/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
Logger::Logger(const char* tag, uint8_t logLevel) {
    _tag = tag;
    _logLevel = logLevel;
}

/******************************************************************************/
/*!
  @brief    Sets the tag prefix of the log.
  @param    tag                 Tag to set
*/
/******************************************************************************/
void Logger::setTag(const char* tag) {
    _tag = tag;
}

/******************************************************************************/
/*!
  @brief    Sets the log level of the logs that are printed.
  @param    tag                 loglevel
*/
/******************************************************************************/
void Logger::setLogLevel(uint8_t logLevel) {
    _logLevel = logLevel;
}
#pragma endregion

#pragma region Main functionality
/******************************************************************************/
/*!
  @brief    Logs a debug line.
  @param    logString           String to log
*/
/******************************************************************************/
void Logger::logd(const char* logString) {
    _log(logString, LOG_TYPE_INFO, false);

    if (_logLevel > LOG_TYPE_DEBUG) {
        return;
    }
    
    Serial.printf("DEBUG: %s: %s\n", _tag, logString);
}

/******************************************************************************/
/*!
  @brief    Logs a debug line.
  @param    logString           String to log
*/
/******************************************************************************/
void Logger::logd(String logString) {
    logd(logString.c_str());
}

/******************************************************************************/
/*!
  @brief    Logs an info line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::logi(const char* logString, bool saveToFile) {
    _log(logString, LOG_TYPE_INFO, saveToFile);

    if (_logLevel > LOG_TYPE_INFO) {
        return;
    }

    Serial.printf("NOTE: %s: %s\n", _tag, logString);
}

/******************************************************************************/
/*!
  @brief    Logs an info line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::logi(String logString, bool saveToFile) {
    logi(logString.c_str(), saveToFile);
}

/******************************************************************************/
/*!
  @brief    Logs a warning line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::logw(const char* logString, bool saveToFile) {
    _log(logString, LOG_TYPE_WARNING, saveToFile);

    if (_logLevel > LOG_TYPE_WARNING) {
        return;
    }

    Serial.printf("WARNING: %s: %s\n", _tag, logString);
}

/******************************************************************************/
/*!
  @brief    Logs a warning line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::logw(String logString, bool saveToFile) {
    logw(logString.c_str(), saveToFile);
}

/******************************************************************************/
/*!
  @brief    Logs an error line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::loge(const char* logString, bool saveToFile) {
    _log(logString, LOG_TYPE_ERROR, saveToFile);

    if (_logLevel > LOG_TYPE_ERROR) {
        return;
    }

    Serial.printf("ERROR: %s: %s\n", _tag, logString);
}

/******************************************************************************/
/*!
  @brief    Logs an error line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::loge(String logString, bool saveToFile) {
    loge(logString.c_str(), saveToFile);
}

/******************************************************************************/
/*!
  @brief    Logs a fatal error line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::logfe(const char* logString, bool saveToFile) {
    _log(logString, LOG_TYPE_FATAL_ERROR, saveToFile);

    if (_logLevel > LOG_TYPE_FATAL_ERROR) {
        return;
    }

    Serial.printf("FATAL ERROR: %s: %s\n", _tag, logString);
}

/******************************************************************************/
/*!
  @brief    Logs a fatal error line.
  @param    logString           String to log
  @param    saveToFile          When true, logs gets saved to the log file
*/
/******************************************************************************/
void Logger::logfe(String logString, bool saveToFile) {
    logfe(logString.c_str(), saveToFile);
}
#pragma endregion

#pragma region Utilities
/******************************************************************************/
/*!
  @brief    Generates a JSON line of the log
  @param    type                Type of log (debug, info, etc.)
  @param    log                 Log line
  @returns  String              JSON string
*/
/******************************************************************************/
String Logger::generateJsonLog(uint8_t type, const char* log) {
    String date;
    String time;
    
    //if (currentDateTime.isConfigured()) {
    //    date = currentDateTime.toDateString();
    //    time = currentDateTime.toTimeString();
    //}

    String jsonLog = "{\"type\":";
    jsonLog += String(type) + ",";
    jsonLog += "\"log\":";
    jsonLog += "\"" + String(log) + "\",";
    jsonLog += "\"date\":";
    jsonLog += "\"" + date + "\",";
    jsonLog += "\"time\":";
    jsonLog += "\"" + time + "\"}\n";

    return jsonLog;
}

/******************************************************************************/
/*!
  @brief    Marks logs as read by moving logs into the logs history file.
*/
/******************************************************************************/
void Logger::markLogsAsRead() {
    /* Clear logs file */
    SD_MMC.remove(_joinPaths(LOGS_DIRECTORY, LOGS_FILE));
    File logsFile = SD_MMC.open(_joinPaths(LOGS_DIRECTORY, LOGS_FILE), FILE_WRITE);
    logsFile.close();

    _numberOfLogs = 0;
    
    logi("Marked logs as read");
}

/******************************************************************************/
/*!
  @brief    Saves the log to the log file.
  @param    log                 Log line
  @param    type                Type of log (debug, info, etc.)
  @param    saveToFile          If true, log line gets saved as JSON
*/
/******************************************************************************/
void Logger::_log(const char* logString, uint8_t type, bool saveToFile) {
    /* If log doesn't need to be saved, return */
    if (!saveToFile) {
        return;
    }

    if (_numberOfLogs >= MAX_NUMBER_OF_LOGS) {
        markLogsAsRead();
    }

    String jsonLog = generateJsonLog(type, logString);

    File file = SD_MMC.open(_joinPaths(LOGS_DIRECTORY, LOGS_FILE), FILE_APPEND);

    if (!file.print(jsonLog)) {
        Serial.printf("ERROR: Couldn't write to logs file\n");
    } else {
        _numberOfLogs++;
    }

    file.close();
}

/******************************************************************************/
/*!
  @brief    Joins multiple paths portions into one seperated by a '/'.
  @param    path1               First portion of the path
  @param    path2               Second portion of the path
  @param    path3               Optional third portion of the path
  @returns  String              Concatenated path
*/
/******************************************************************************/
String Logger::_joinPaths(String path1, String path2, String path3) {
    if (path3 == "") {
        return path1 + "/" + path2;
    } else {
        return path1 + "/" + path2 + "/" + path3;
    }
}
#pragma endregion