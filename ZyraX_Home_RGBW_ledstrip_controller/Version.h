/******************************************************************************/
/*
 * File:    Version.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Version encapsulation class for handling semantic versions.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef VERSION_H
#define VERSION_H
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "stdint.h"                                                             //For size defined int types

#define NUMBER_OF_SEPARATORS_IN_VERSION     2

class Version {
    public:
        Version(String string = "");

        void setVersion(uint8_t major, uint8_t minor = 0, uint8_t patch = 0);
        bool setVersion(String string);

        String getVersionUnderscoreString();
        String getVersionDottedString();
        
    private:
        uint8_t _major = 0;
        uint8_t _minor = 0;
        uint8_t _patch = 0;
        String _underScoreString;
        String _dottedString;
};
#endif