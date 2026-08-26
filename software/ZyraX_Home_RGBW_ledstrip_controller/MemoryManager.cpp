/******************************************************************************/
/*
 * File:    MemoryManager.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   MemoryManager class for handling most of the SPIFFS card filesystem
 *          interaction. Main tasks are reading files.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#include "MemoryManager.h"

#pragma region Main class functionality
/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
MemoryManager::MemoryManager() {
    _l.setTag("MemoryManager");
}

/******************************************************************************/
/*!
  @brief    Initializes the micro SPIFFS card reader and mounts it.
  @returns  bool                True if SPIFFS card is mounted
*/
/******************************************************************************/
bool MemoryManager::initialize() {
    _l.logi("Mounting SPI Flash File System", false);

    /* Initialize SPIFFS, (true ==) format if mount failed */
    if (!SPIFFS.begin(true)) {
        _l.loge("Failed to mount SPIFFS, please check it or reinstall the firmware");
        return false;
    }

    _sdMounted = false;
    
    SD_MMC.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_DATA_PIN);

    uint8_t retry = 10;

    /* Try to mount SD card*/
    while (retry > 0) {
        if (SD_MMC.begin("/sd", true, false, BOARD_MAX_SDMMC_FREQ, 7)) {
            _sdMounted = true;
            break;
        }

        retry--;
        vTaskDelay(100);
        _l.logw("SD not connected, trying again", false);
    }

    if (!_sdMounted) {
        _l.logfe("Failed to mount SD card, check SD card", false);
        return false;
    }
  
    if (SD_MMC.cardType() == CARD_NONE) {
        _l.logfe("No SD card attached");
        return false;
    }

    if (!_checkFileSystemStructure()) {
        return false;
    }

    return true;
}

/******************************************************************************/
/*!
  @brief    Loads the mode parameters for the specified mode.
  @param    mode                Mode ID
  @returns  ModeParameters      Configuration struct
*/
/******************************************************************************/
ModeParameters MemoryManager::loadModeParameters(uint8_t mode) {
    ModeParameters parameters;
    String modeString = String(mode);

    /* Load states from non-volatile memory (names maximum 16 chars) */
    _nvMemory.begin(NV_MEM_CONFIG);
    
    parameters.minColorPos = _nvMemory.getUChar(String("minColorPos_" + modeString).c_str(), 0);
    parameters.maxColorPos = _nvMemory.getUChar(String("maxColorPos_" + modeString).c_str(), 255);
    parameters.color1 = _stringToCRGB(_nvMemory.getString(String("color1_" + modeString).c_str(), "(255,255,255)"));
    parameters.color2 = _stringToCRGB(_nvMemory.getString(String("color2_" + modeString).c_str(), "(0,0,0)"));
    parameters.useGradient1 = _nvMemory.getBool(String("useGradient1_" + modeString).c_str(), false);
    parameters.useGradient2 = _nvMemory.getBool(String("useGradient2_" + modeString).c_str(), false);
    parameters.segmentSize = _nvMemory.getUChar(String("segmentSize_" + modeString).c_str(), 2);
    parameters.tailLength = _nvMemory.getUChar(String("tailLength_" + modeString).c_str(), 0);
    parameters.waveLength = _nvMemory.getUChar(String("waveLength_" + modeString).c_str(), 10);
    parameters.timeFade = _nvMemory.getUShort(String("timeFade_" + modeString).c_str(), 100);
    parameters.delay = _nvMemory.getUShort(String("delay_" + modeString).c_str(), 500);
    parameters.delayBetween = _nvMemory.getUShort(String("delayBetween_" + modeString).c_str(), 100);
    parameters.randomnessDelay = _nvMemory.getUChar(String("randomDelay_" + modeString).c_str(), 0);
    parameters.intensity = _nvMemory.getUShort(String("intensity_" + modeString).c_str(), 1);
    parameters.direction = _nvMemory.getUShort(String("direction_" + modeString).c_str(), DIRECTION_LEFT);
    parameters.numberOfElements = _nvMemory.getUShort(String("numElems_" + modeString).c_str(), 1);
    parameters.palette = _nvMemory.getUShort(String("palette_" + modeString).c_str(), PALETTE_RANDOM);
    parameters.fadeLength = _nvMemory.getUShort(String("fadeLength_" + modeString).c_str(), 100);

    _nvMemory.end();

    return parameters;
}

/******************************************************************************/
/*!
  @brief    Generates a JSON string based on the specified mode parameter.
  @param    parameterId         ID of the parameter
  @param    parameterValue      Value of the parameter
  @param    isString            True if value is string
  @returns  String              Parameter JSON string
*/
/******************************************************************************/
String MemoryManager::_generateModeParameterJsonString(uint8_t parameterId, String parameterValue1, bool isString1, String parameterValue2) {
    String jsonString = "{\"id\":\"" + String(parameterId) + "\", ";
    if (isString1) {
        jsonString += "\"value1\":\"" + String(parameterValue1) + "\"";
    } else {
        jsonString += "\"value1\":" + String(parameterValue1);
    }

    if (parameterValue2 != "") {
        jsonString += ",\"value2\":" + String(parameterValue2) + "}";
    } else {
        jsonString += "}";
    }

    return jsonString;
}

/******************************************************************************/
/*!
  @brief    Returns the JSON string of the specified mode.
  @param    mode                Mode ID
  @returns  String              JSON string of the mode
*/
/******************************************************************************/
String MemoryManager::getModeJsonString(uint8_t mode) {
    ModeParameters parameters = loadModeParameters(mode);
    String jsonString = "{\"mode\":" + String(mode) + ", \"parameters\": [";

    bool firstParameter = true;

    auto addParameter = [&](String parameter) {
        if (!firstParameter) {
            jsonString += ",";
        }

        jsonString += parameter;
        firstParameter = false;
    };

    if (modeHasParameter(mode, PARAMETER_ID_COLOR_RANGE)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_COLOR_RANGE,
                String(parameters.minColorPos),
                false,
                String(parameters.maxColorPos)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_COLOR1)) {
        char colorString[8] = {0};
        sprintf(colorString, "#%06x", rgbToHex(parameters.color1));

        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_COLOR1,
                String(colorString),
                true,
                String(parameters.useGradient1)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_COLOR2)) {
        char colorString[8] = {0};
        sprintf(colorString, "#%06x", rgbToHex(parameters.color2));

        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_COLOR2,
                String(colorString),
                true,
                String(parameters.useGradient2)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_SEGMENT_SIZE)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_SEGMENT_SIZE,
                String(parameters.segmentSize)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_TAIL_LENGTH)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_TAIL_LENGTH,
                String(parameters.tailLength)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_WAVE_LENGTH)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_WAVE_LENGTH,
                String(parameters.waveLength)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_TIME_FADE)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_TIME_FADE,
                String(parameters.timeFade)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_DELAY)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_DELAY,
                String(parameters.delay)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_DELAY_BETWEEN)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_DELAY_BETWEEN,
                String(parameters.delayBetween)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_RANDOMNESS_DELAY)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_RANDOMNESS_DELAY,
                String(parameters.randomnessDelay)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_INTENSITY)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_INTENSITY,
                String(parameters.intensity)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_DIRECTION)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_DIRECTION,
                String(parameters.direction)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_NUMBER_OF_ELEMENTS)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_NUMBER_OF_ELEMENTS,
                String(parameters.numberOfElements)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_PALETTE)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_PALETTE,
                String(parameters.palette)
            )
        );
    }

    if (modeHasParameter(mode, PARAMETER_ID_FADE_LENGTH)) {
        addParameter(
            _generateModeParameterJsonString(
                PARAMETER_ID_FADE_LENGTH,
                String(parameters.fadeLength)
            )
        );
    }

    jsonString += "]}";

    return jsonString;
}

/******************************************************************************/
/*!
  @brief    Saved the specified mode parameters to the non-volatile memory.
  @param    mode                Mode ID
  @param    parameters          Mode parameters to write
*/
/******************************************************************************/
void MemoryManager::writeModeParameters(uint8_t mode, ModeParameters parameters) {
    String modeString = String(mode);

    /* Load states from non-volatile memory (names maximum 16 chars) */
    _nvMemory.begin(NV_MEM_CONFIG);

    if (modeHasParameter(mode, PARAMETER_ID_COLOR_RANGE)) {
        _nvMemory.putUChar(String("minColorPos_" + modeString).c_str(), parameters.minColorPos);
        _nvMemory.putUChar(String("maxColorPos_" + modeString).c_str(), parameters.maxColorPos);
    }
    if (modeHasParameter(mode, PARAMETER_ID_COLOR1)) {
        _nvMemory.putString(String("color1_" + modeString).c_str(), _crgbToString(parameters.color1));
        _nvMemory.putBool(String("useGradient1_" + modeString).c_str(), parameters.useGradient1);
    }
    if (modeHasParameter(mode, PARAMETER_ID_COLOR2)) {
        _nvMemory.putString(String("color2_" + modeString).c_str(), _crgbToString(parameters.color2));
        _nvMemory.putBool(String("useGradient2_" + modeString).c_str(), parameters.useGradient2);
    }
    if (modeHasParameter(mode, PARAMETER_ID_SEGMENT_SIZE)) {
        _nvMemory.putUChar(String("segmentSize_" + modeString).c_str(), parameters.segmentSize);
    }
    if (modeHasParameter(mode, PARAMETER_ID_TAIL_LENGTH)) {
        _nvMemory.putUChar(String("tailLength_" + modeString).c_str(), parameters.tailLength);
    }
    if (modeHasParameter(mode, PARAMETER_ID_WAVE_LENGTH)) {
        _nvMemory.putUChar(String("waveLength_" + modeString).c_str(), parameters.waveLength);
    }
    if (modeHasParameter(mode, PARAMETER_ID_TIME_FADE)) {
        _nvMemory.putUShort(String("timeFade_" + modeString).c_str(), parameters.timeFade);
    }
    if (modeHasParameter(mode, PARAMETER_ID_DELAY)) {
        _nvMemory.putUShort(String("delay_" + modeString).c_str(), parameters.delay);
    }
    if (modeHasParameter(mode, PARAMETER_ID_DELAY_BETWEEN)) {
        _nvMemory.putUShort(String("delayBetween_" + modeString).c_str(), parameters.delayBetween);
    }
    if (modeHasParameter(mode, PARAMETER_ID_RANDOMNESS_DELAY)) {
        _nvMemory.putUChar(String("randomDelay_" + modeString).c_str(), parameters.randomnessDelay);
    }
    if (modeHasParameter(mode, PARAMETER_ID_INTENSITY)) {
        _nvMemory.putUShort(String("intensity_" + modeString).c_str(), parameters.intensity);
    }
    if (modeHasParameter(mode, PARAMETER_ID_DIRECTION)) {
        _nvMemory.putUShort(String("direction_" + modeString).c_str(), parameters.direction);
    }
    if (modeHasParameter(mode, PARAMETER_ID_NUMBER_OF_ELEMENTS)) {
        _nvMemory.putUShort(String("numElems_" + modeString).c_str(), parameters.numberOfElements);
    }
    if (modeHasParameter(mode, PARAMETER_ID_PALETTE)) {
        _nvMemory.putUShort(String("palette_" + modeString).c_str(), parameters.palette);
    }
    if (modeHasParameter(mode, PARAMETER_ID_FADE_LENGTH)) {
        _nvMemory.putUShort(String("fadeLength_" + modeString).c_str(), parameters.fadeLength);
    }
    
    _nvMemory.end();
}

/******************************************************************************/
/*!
  @brief    Creates a folder when the folder doesn't yet exist.
  @param    path                Path of the folder
  @returns  bool                True if successful
*/
/******************************************************************************/
bool MemoryManager::createFolderIfNotExists(String path) {
    File root = SD_MMC.open(path);

    if (root.isDirectory()) {
        return true;
    }

    if (!SD_MMC.mkdir(path)) {
        _l.loge("Cannot create [" + path + "]");
        return false;
    }

    _l.logi("Created [" + path + "]");
    return true;
}

/******************************************************************************/
/*!
  @brief    Creates a folder when the folder doesn't yet exist.
  @param    path                Path of the folder
  @returns  bool                True if successful
*/
/******************************************************************************/
bool MemoryManager::getSdMounted() {
    return _sdMounted;
}
#pragma endregion

#pragma region Reset functionality
/******************************************************************************/
/*!
  @brief    Resets all the non-volatile variables.
*/
/******************************************************************************/
void MemoryManager::resetNVvariables() {
    _nvMemory.begin(NV_MEM_CONFIG);
    _nvMemory.clear();
    _nvMemory.end();

    _l.logi("Resetted NV memory");
}
#pragma endregion

#pragma region Utilities
/******************************************************************************/
/*!
  @brief    Copies the specified file.
  @param    filePath            Path of the file to copy
  @param    copyPath            Path of the new file create
  @returns  bool                True if successful
*/
/******************************************************************************/
bool MemoryManager::copyFile(String filePath, String copyPath) {
    if (SD_MMC.exists(copyPath)) {
        _l.logw("File to copy already exists, removing it", false);
        SD_MMC.remove(copyPath);
    }

    File copiedFile = SD_MMC.open(copyPath, FILE_WRITE);

    if (!copiedFile) {
        _l.loge("File could not be copied");
        return false;
    }

    File fileToCopy = SD_MMC.open(filePath, FILE_READ);

    while (fileToCopy.available()) {
        copiedFile.write(fileToCopy.read());
    }

    fileToCopy.close();
    copiedFile.close();

    return true;
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
String MemoryManager::joinPaths(String path1, String path2, String path3) {
    if (path3 == "") {
        return path1 + "/" + path2;
    } else {
        return path1 + "/" + path2 + "/" + path3;
    }
}

/******************************************************************************/
/*!
  @brief    Checks whether all required directories and files are present.
  @returns  bool                True if file system structure is OK
*/
/******************************************************************************/
bool MemoryManager::_checkFileSystemStructure() {
    /* Just check the 'deepest' folders */
    if (!_checkDirectory(SYSTEM_DIRECTORY)) {
        return false;
    }

    if (!_checkDirectory(LOGS_DIRECTORY)) {
        return false;
    }

    if (!_checkDirectory(OTA_FIRMWARE_DIRECTORY)) {
        return false;
    }

    /* Check files */
    if (!_checkFile(joinPaths(LOGS_DIRECTORY, LOGS_FILE))) {
        return false;
    }

    return true;
}

/******************************************************************************/
/*!
  @brief    Checks whether the specified directory is present. If not present,
            creates the directory.
  @param    directory           Directory to check
  @returns  bool                True if directory is OK
*/
/******************************************************************************/
bool MemoryManager::_checkDirectory(String directory) {
    File root;

    root = SD_MMC.open(directory);
    if (!root.isDirectory()) {
        _l.logw("Directory [" + directory + "] not found, creating it");

        if (!SD_MMC.mkdir(directory)) {
            _l.logfe("Directory [" + directory + "] could not be created");
            return false;
        }
    }

    return true;
}

/******************************************************************************/
/*!
  @brief    Checks whether the specified file is present. If not present,
            creates the file.
  @param    filePath            File to check
  @param    defaultFilePath     When the file is not present and this is
                                specified, the file will be copied from this
                                location. Else creats an empty file
  @returns  bool                True if file is OK
*/
/******************************************************************************/
bool MemoryManager::_checkFile(String filePath, String defaultFilePath) {
    if (SD_MMC.exists(filePath)) {
        return true;
    }

    if (defaultFilePath != "") {
        _l.logw("File [" + filePath + "] not found, restoring default file");
        if (!copyFile(defaultFilePath, filePath)) {
            _l.logfe("File [" + filePath + "] could not be restored");
            return false;
        }

        return true;
    }

    /* If no default file path is included, just create an empty file */
    _l.logw("File [" + filePath + "] not found, creating it");
    File file = SD_MMC.open(filePath, FILE_WRITE);

    if (!file) {
        _l.logfe("File [" + filePath + "] could not be created");
        return false;
    }

    file.close();
    return true;
}
#pragma endregion

/******************************************************************************/
/*!
  @brief    Converts the specified color string to a CRGB object.
  @param    colorString         Color string, format (xxx,xxx,xxx)
  @returns  CRGB                Color object
*/
/******************************************************************************/
CRGB MemoryManager::_stringToCRGB(String colorString) {
    CRGB color = CRGB(0, 0, 0);
    if (colorString.length() < 7 || colorString[0] != '(') {
        _l.logw("Stored color has an invalid format");
        return color;
    }
    uint8_t index = 1;                                                          //'('
    uint8_t c = 1;
    
    for (uint8_t channel = 0; channel < 3; channel++) {
        while (c < colorString.length() && c < 16 && colorString[c] != ',' && colorString[c] != ')') {
            c++;
        }
        if (c >= colorString.length() || c >= 16 || c == index) {
            _l.logw("Stored color has an invalid format");
            return CRGB(0, 0, 0);
        }
        int32_t channelValue = colorString.substring(index, c).toInt();
        color[channel] = constrain(channelValue, 0, 255);
        c++;
        index = c;
    }
    
    return color;
}

/******************************************************************************/
/*!
  @brief    Converts the specified CRGB object to a color string.
  @param    color               Color object
  @returns  String              Color string, format (xxx,xxx,xxx)
*/
/******************************************************************************/
String MemoryManager::_crgbToString(CRGB color) {
    String colorString = "(";
    colorString += String(color.r);
    colorString += ",";
    colorString += String(color.g);
    colorString += ",";
    colorString += String(color.b);
    colorString += ")";

    return colorString;
}

/******************************************************************************/
/*!
  @brief    Converts the specified CRGB object to a HEX color.
  @param    color               Color object
  @returns  uint32_t            HEX color
*/
/******************************************************************************/
uint32_t MemoryManager::rgbToHex(CRGB color) {
    return ((color.r & 0xff) << 16) + ((color.g & 0xff) << 8) + (color.b & 0xff);
}

/******************************************************************************/
/*!
  @brief    Returns the number of mode parameters for the specified mode.
  @param    mode                Mode ID
  @returns  uint8_t             Number of parameters
*/
/******************************************************************************/
uint8_t MemoryManager::getnumberOfModeParameters(uint8_t mode) {
    uint8_t parameterIds[] = {
        PARAMETER_ID_COLOR_RANGE,
        PARAMETER_ID_COLOR1,
        PARAMETER_ID_COLOR2,
        PARAMETER_ID_SEGMENT_SIZE,
        PARAMETER_ID_TAIL_LENGTH,
        PARAMETER_ID_WAVE_LENGTH,
        PARAMETER_ID_TIME_FADE,
        PARAMETER_ID_DELAY,
        PARAMETER_ID_DELAY_BETWEEN,
        PARAMETER_ID_RANDOMNESS_DELAY,
        PARAMETER_ID_INTENSITY,
        PARAMETER_ID_DIRECTION,
        PARAMETER_ID_NUMBER_OF_ELEMENTS,
        PARAMETER_ID_PALETTE,
        PARAMETER_ID_FADE_LENGTH
    };

    uint8_t numberOfParameters = 0;
    for (uint8_t i = 0; i < NUMBER_OF_MODE_PARAMETERS; i++) {
        if (modeHasParameter(mode, parameterIds[i])) {
            numberOfParameters++;
        }
    }

    return numberOfParameters;
}

/******************************************************************************/
/*!
  @brief    Returns wether the specified mode has the specified mode parameter.
  @param    mode                Mode ID
  @param    parameterId         ID of the parameter
  @returns  bool                True if mode has the specified parameter
*/
/******************************************************************************/
bool MemoryManager::modeHasParameter(uint8_t mode, uint8_t parameterId) {
    switch (mode) {
        case MODE_COLOR:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            return false;

        case MODE_FADE:
            if (parameterId == PARAMETER_ID_COLOR_RANGE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            return false;

        case MODE_GRADIENT:
            if (parameterId == PARAMETER_ID_COLOR_RANGE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_WAVE_LENGTH) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            return false;

        case MODE_BLINK:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            return false;

        case MODE_SCAN:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_SEGMENT_SIZE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_TAIL_LENGTH) {
                return true;
            }
            return false;

        case MODE_THEATER:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DIRECTION) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_SEGMENT_SIZE) {
                return true;
            }
            return false;

        case MODE_SINE:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DIRECTION) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_WAVE_LENGTH) {
                return true;
            }
            return false;

        case MODE_BOUNCING_BALLS:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_NUMBER_OF_ELEMENTS) {
                return true;
            }
            if (parameterId == PARAMETER_ID_SEGMENT_SIZE) {
                return true;
            }
            return false;

        case MODE_DISSOLVE:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_TIME_FADE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY_BETWEEN) {
                return true;
            }
            return false;

        case MODE_SPARKLE:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_INTENSITY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY_BETWEEN) {
                return true;
            }
            if (parameterId == PARAMETER_ID_TIME_FADE) {
                return true;
            }
            return false;

        case MODE_FIREWORKS:
            if (parameterId == PARAMETER_ID_PALETTE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY_BETWEEN) {
                return true;
            }
            if (parameterId == PARAMETER_ID_RANDOMNESS_DELAY) {
                return true;
            }
            return false;
        
        case MODE_FIRE:
            if (parameterId == PARAMETER_ID_PALETTE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_SEGMENT_SIZE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            return false;
        
        case MODE_SWEEP:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_COLOR2) {
                return true;
            }
            if (parameterId == PARAMETER_ID_FADE_LENGTH) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY_BETWEEN) {
                return true;
            }
            return false;

        case MODE_COLOR_TWINKELS:
            if (parameterId == PARAMETER_ID_PALETTE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_TIME_FADE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY_BETWEEN) {
                return true;
            }
            return false;
            
        case MODE_METEOR_RAIN:
            if (parameterId == PARAMETER_ID_COLOR1) {
                return true;
            }
            if (parameterId == PARAMETER_ID_SEGMENT_SIZE) {
                return true;
            }
            if (parameterId == PARAMETER_ID_TAIL_LENGTH) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY) {
                return true;
            }
            if (parameterId == PARAMETER_ID_DELAY_BETWEEN) {
                return true;
            }
            if (parameterId == PARAMETER_ID_RANDOMNESS_DELAY) {
                return true;
            }
            return false;

        case MODE_COLOR_WAVES:
            if (parameterId == PARAMETER_ID_PALETTE) {
                return true;
            }
            return false;

        case MODE_TEMPLATE_1:
            return false;
        case MODE_TEMPLATE_2:
            return false;
        case MODE_TEMPLATE_3:
            return false;
        case MODE_TEMPLATE_4:
            return false;
        case MODE_TEMPLATE_5:
            return false;
        case MODE_TEMPLATE_6:
            return false;
            
        case MODE_TEMPLATE_7:
            return false;
        
        case MODE_TEMPLATE_8:
            return false;
            
        case MODE_TEMPLATE_9:
            return false;
        
        case MODE_TEMPLATE_10:
            return false;
        default:
            return false;
    }
}
