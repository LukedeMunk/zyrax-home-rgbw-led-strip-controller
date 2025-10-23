/******************************************************************************/
/*
 * File:    DateTime.h
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
#ifndef DATETIME_H
#define DATETIME_H
#include "Arduino.h"                                                            //For additional Arduino framework features, like String object
#include "stdint.h"                                                             //For size defined int types
#include "time.h"                                                               //For POSIX time functionality

/* Days */
#define MONDAY                          0
#define TUESDAY                         1
#define WEDNESDAY                       2
#define THURSDAY                        3
#define FRIDAY                          4
#define SATURDAY                        5
#define SUNDAY                          6

#define MINUTES_IN_HOUR                 60

struct DateTimeStruct {
    uint8_t weekDay = MONDAY;
    uint8_t day = 1;
    uint8_t month = 1;
    uint16_t year = 1970;
    
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
};

class DateTime {
    public:
        DateTime();

        /* Main functionality */
        tm getDateTime();                                                       //NOT YET USED
        time_t getTime();
        uint8_t getWeek();
        bool isConfigured();
        void setDateTime(tm dateTime);
        void setDateTime(String dateTimeString);
        void setOffset(int8_t offset);

        /* Conversion functionality */
        String toDateString();
        String toTimeString(bool returnSeconds = true);
        String toDateTimeString();

        /* Calculation functionality */
        bool compareTimes(DateTimeStruct time1, DateTimeStruct time2);          //NOT YET USED
        uint16_t getMinutesBetween(DateTimeStruct time1, DateTimeStruct time2); //NOT YET USED
        uint16_t getMinutesBetween(time_t time2);                               //NOT YET USED
        uint16_t getHoursBetween(time_t time2);                                 //NOT YET USED
        uint16_t getDaysBetween(time_t time2);                                  //NOT YET USED
        
    private:
        time_t _now;
        tm _timeStruct;
        char _stringBuffer[32];
        int8_t _timeOffset;

        bool _isConfigured;
};
#endif
