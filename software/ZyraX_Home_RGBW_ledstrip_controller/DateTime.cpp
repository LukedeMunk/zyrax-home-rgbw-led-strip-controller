/******************************************************************************/
/*
 * File:    DateTime.cpp
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Class for date and time functionality. Makes use of the POSIX
 *          functions and provides extended functionality.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#include "DateTime.h"

#pragma region Main class functionality
/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
DateTime::DateTime() {
    _isConfigured = false;                                                      //Time is volatile. Needs to be configured on every power cycle
    _timeOffset = 0;
}

/******************************************************************************/
/*!
  @brief    Returns the date and time in POSIX tm format.
  @returns  tm                  Date and time in POSIX tm struct
*/
/******************************************************************************/
tm DateTime::getDateTime() {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct

    return _timeStruct;
}

/******************************************************************************/
/*!
  @brief    Returns the date and time in time_t format.
  @returns  time_t              Date and time in time_t struct
*/
/******************************************************************************/
time_t DateTime::getTime() {
    return _now;
}

/******************************************************************************/
/*!
  @brief    Returns the current week number.
  @returns  uint8_t             Current week number
*/
/******************************************************************************/
uint8_t DateTime::getWeek() {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct
    strftime(_stringBuffer, sizeof(_stringBuffer), "%V", &_timeStruct);         //Print date in format: ISO_8601
    return atoi(_stringBuffer);
}

/******************************************************************************/
/*!
  @brief    Returns whether the date and time is configured.
  @return   bool                True if date and time is configured
*/
/******************************************************************************/
bool DateTime::isConfigured() {
    return _isConfigured;
}

/******************************************************************************/
/*!
  @brief    Sets the date and time by a POSIX tm variable.
  @param	dateTime            Date and time to set
*/
/******************************************************************************/
void DateTime::setDateTime(tm dateTime) {
    dateTime.tm_hour += _timeOffset;                                            //Add time offset
    _now = mktime(&dateTime);                                                   //Create time type from struct
    
    timeval timeValue;                                                          //Create timeval object to feed settimeofday() function
    timeValue.tv_sec = _now;                                                    //Assign time here to this object.
    timeValue.tv_usec = 0;                                                      //time_t can hold only seconds.
    settimeofday(&timeValue, NULL);

    _isConfigured = true;
    Serial.println("NOTE: DateTime: Date and time configured");
}

/******************************************************************************/
/*!
  @brief    Sets the date and time by a date and time string.
  @param    dateTimeString      Date and time to set. Format: DD-MM-YYYY_HH:MM:SS
*/
/******************************************************************************/
void DateTime::setDateTime(String dateTimeString) {
    strptime(dateTimeString.c_str(), "%d-%m-%Y_%H:%M:%S", &_timeStruct);        //Convert string to tm struct
    setDateTime(_timeStruct);
}

/******************************************************************************/
/*!
  @brief    Sets the time offset.
  @param    offset              Offset in hours
*/
/******************************************************************************/
void DateTime::setOffset(int8_t offset) {
    /* If is configured, update date and time. Else, just save offset */
    if (_isConfigured) {
        localtime_r(&_now, &_timeStruct);                                       //Convert time since epoch to calendar time and save in struct
        _timeStruct.tm_hour -= _timeOffset;                                     //Reset offset
        _timeOffset = offset;
        setDateTime(_timeStruct);
    } else {
        _timeOffset = offset;
    }
}
#pragma endregion

#pragma region Conversion functionality
/******************************************************************************/
/*!
  @brief    Returns the date in string format.
  @returns  String              Current date. Format: DD-MM-YYYY
*/
/******************************************************************************/
String DateTime::toDateString() {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct
    strftime(_stringBuffer, sizeof(_stringBuffer), "%d-%m-%Y", &_timeStruct);   //Print date in format: ISO_8601

    return String(_stringBuffer);
}

/******************************************************************************/
/*!
  @brief    Returns the time in string format.
  @param    returnSeconds       If true, seconds are included
  @returns  String              Current time. Format: HH:MM(:SS)
*/
/******************************************************************************/
String DateTime::toTimeString(bool returnSeconds) {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct
    if (returnSeconds) {
        strftime(_stringBuffer, sizeof(_stringBuffer), "%H:%M:%S", &_timeStruct);   //Print time in format: ISO_8601
    } else {
        strftime(_stringBuffer, sizeof(_stringBuffer), "%H:%M", &_timeStruct);      //Print time in format: ISO_8601
    }

    return String(_stringBuffer);
}

/******************************************************************************/
/*!
  @brief    Returns the date and time in string format.
  @returns  String              Current date and time.
                                Format: DD-MM-YYYY_HH:MM:SS
*/
/******************************************************************************/
String DateTime::toDateTimeString() {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct
    strftime(_stringBuffer, sizeof(_stringBuffer), "%d-%m-%Y_%H:%M:%S", &_timeStruct);  //Print date time in format: ISO_8601

    return String(_stringBuffer);
}
#pragma endregion

#pragma region Calculation functionality
/******************************************************************************/
/*!
  @brief    Returns whether the two specified dates and times are equal or not.
  @param    time1               Date and time 1
  @param    time2               Date and time 2
  @returns  bool                True if the two dates are equal
*/
/******************************************************************************/
bool DateTime::compareTimes(DateTimeStruct time1, DateTimeStruct time2) {
    bool isSame = true;
    
    if (time1.hour != time2.hour) {
        time1.hour = time2.hour;
        isSame = false;
    }

    if (time1.minute != time2.minute) {
        time1.minute = time2.minute;
        isSame = false;
    }

    return isSame;
}

/******************************************************************************/
/*!
  @brief    Returns the number of minutes between two specified dates and times.
  @param    time1               Date and time 1
  @param    time2               Date and time 2
  @returns  uint16_t            Minutes between two specified dates
*/
/******************************************************************************/
uint16_t DateTime::getMinutesBetween(DateTimeStruct time1, DateTimeStruct time2) {
    uint16_t minutes1 = time1.minute + time1.hour * MINUTES_IN_HOUR;
    uint16_t minutes2 = time2.minute + time2.hour * MINUTES_IN_HOUR;

    return abs(minutes1 - minutes2);
}

/******************************************************************************/
/*!
  @brief    Returns the number of minutes between the specified date and time
            and the current date and time.
            IMPORTANT: Only hours and minutes are taken in consideration.
  @param    time2               Date and time 2
  @returns  uint16_t            Minutes between
*/
/******************************************************************************/
uint16_t DateTime::getMinutesBetween(time_t time2) {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct

    tm timeStruct2;
    localtime_r(&time2, &timeStruct2);                                          //Save in struct
    
    uint8_t hours = abs(_timeStruct.tm_hour - timeStruct2.tm_hour);
    return abs(_timeStruct.tm_min - timeStruct2.tm_min) + hours * MINUTES_IN_HOUR;
}

/******************************************************************************/
/*!
  @brief    Returns the number of hours between the specified date and time
            and the current date and time.
  @param    time2               Date and time 2
  @returns  uint16_t            Hours between
*/
/******************************************************************************/
uint16_t DateTime::getHoursBetween(time_t time2) {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct

    tm timeStruct2;
    localtime_r(&time2, &timeStruct2);                                          //Convert time since epoch to calendar time and save in struct

    return abs(_timeStruct.tm_hour - timeStruct2.tm_hour);
}

/******************************************************************************/
/*!
  @brief    Returns the number of days between the specified date and time
            and the current date and time.
  @param    time2               Date and time 2
  @returns  uint16_t            Days between
*/
/******************************************************************************/
uint16_t DateTime::getDaysBetween(time_t time2) {
    time(&_now);                                                                //Update time
    localtime_r(&_now, &_timeStruct);                                           //Convert time since epoch to calendar time and save in struct

    tm timeStruct2;
    localtime_r(&time2, &timeStruct2);                                          //Convert time since epoch to calendar time and save in struct

    return abs(_timeStruct.tm_yday - timeStruct2.tm_yday);
}
#pragma endregion