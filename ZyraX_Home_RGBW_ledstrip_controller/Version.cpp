/******************************************************************************/
/*
 * File:    Version.cpp
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Version class for handling version information and checking.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#include "Version.h"

/******************************************************************************/
/*!
  @brief    Constructor.
  @param    string              Version string to set
*/
/******************************************************************************/
Version::Version(String string) {
    if (string != "") {
        setVersion(string);
    }
}

/******************************************************************************/
/*!
  @brief    Sets the version by individual portions.
  @param    major               Major portion
  @param    minor               Minor portion
  @param    patch               Patch portion
*/
/******************************************************************************/
void Version::setVersion(uint8_t major, uint8_t minor, uint8_t patch) {
    _major = major;
    _minor = minor;
    _patch = patch;

    _underScoreString = "v" + String(_major) + "_" + String(_minor) + "_" + String(_patch);
    _dottedString = "v" + String(_major) + "." + String(_minor) + "." + String(_patch);
}

/******************************************************************************/
/*!
  @brief    Sets the version by string.
  @param    string              Version string. Format: (v)X.X.X or (v)X_X_X
  @returns  bool                True if succesfull
*/
/******************************************************************************/
bool Version::setVersion(String string) {
    uint8_t cursor = string.indexOf("v");
    uint8_t nextCursor;

    if (cursor >= 0) {
        cursor++;                                                               //Skip 'v'
    } else {
        cursor = 0;
    }
    
    uint8_t numberOfSeperators = 0;
    char seperator;

    for (uint8_t i = 0; i < string.length(); i++) {
        if (string[i] == '.') {
            numberOfSeperators++;
            seperator = '.';
        }
    }

    if (numberOfSeperators == 0) {
        for (uint8_t i = 0; i < string.length(); i++) {
            if (string[i] == '_') {
                numberOfSeperators++;
                seperator = '_';
            }
        }
    }

    if (numberOfSeperators != NUMBER_OF_SEPARATORS_IN_VERSION) {
        Serial.println("No valid version string");
        return false;
    }

    nextCursor = string.indexOf(seperator);
    String major = string.substring(cursor, nextCursor);
    cursor = nextCursor + 1;                                                    //Skip seperator

    nextCursor = string.indexOf(seperator, cursor);
    String minor = string.substring(cursor, nextCursor);
    cursor = nextCursor + 1;                                                    //Skip seperator
    
    nextCursor = string.indexOf(seperator, cursor);
    String patch = string.substring(cursor, nextCursor);

    setVersion(major.toInt(), minor.toInt(), patch.toInt());

    return true;
}

/******************************************************************************/
/*!
  @brief    Returns the version in string format with underscores.
  @returns  String              Version string
*/
/******************************************************************************/
String Version::getVersionUnderscoreString() {
    return _underScoreString;
}

/******************************************************************************/
/*!
  @brief    Returns the version in string format with dots.
  @returns  String              Version string
*/
/******************************************************************************/
String Version::getVersionDottedString() {
    return _dottedString;
}