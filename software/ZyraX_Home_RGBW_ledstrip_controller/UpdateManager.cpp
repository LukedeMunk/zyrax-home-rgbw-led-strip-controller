/******************************************************************************/
/*
 * File:    UpdateManager.cpp
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
#include "UpdateManager.h"

#pragma region Main class functionality
/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
UpdateManager::UpdateManager() {
    _l.setTag("UpdateManager");
    _ledstripId = 255;
}

/******************************************************************************/
/*!
  @brief    Initializes by loading the state and version. When state is
            installing, it tries again.
*/
/******************************************************************************/
void UpdateManager::initialize(uint8_t id) {
    /* Already initialized */
    if (_ledstripId == id) {
        return;
    }

    /* Get state from non-volatile memory */
    _nvMemory.begin(NV_MEM_CONFIG, true);
    _state = _nvMemory.getUShort("state", OTA_STATE_IDLE);
    _updateToVersion.setVersion(_nvMemory.getString("updateToVer", DEFAULT_VERSION));
    _powerCycles = _nvMemory.getUChar("powerCycles");
    _nvMemory.end();

    _ledstripId = id;

    if (_state != OTA_STATE_IDLE && _state != OTA_STATE_FINISHED) {
        _powerCycles++;

        /* Save state in non-volatile memory */
        _nvMemory.begin(NV_MEM_CONFIG);
        _nvMemory.putUChar("powerCycles", _powerCycles);
        _nvMemory.end();
    }

    /* 
     * Watch the number of power cycles the
     * download/installation process takes
     * to avoid infinite boot looping.
     */
    if (_powerCycles > MAX_NUM_POWER_CYCLES) {
        _state = OTA_STATE_FINISHED;
        _l.loge("OTA update failed, aborting");
    }

    if (_state == OTA_STATE_FINISHED) {
        _l.logi("OTA update succeeded");
        _sendState();
        _state = OTA_STATE_IDLE;
        _powerCycles = 0;

        /* Save state in non-volatile memory */
        _nvMemory.begin(NV_MEM_CONFIG);
        _nvMemory.putUShort("state", _state);
        _nvMemory.putUChar("powerCycles", _powerCycles);
        _nvMemory.end();
    }
}

/******************************************************************************/
/*!
  @brief    Starts the firmware update process of the specified version.
  @param    version             Version to install
*/
/******************************************************************************/
void UpdateManager::updateSystem(Version version) {
    _updateToVersion = version;

    /* Save version in non-volatile memory */
    _nvMemory.begin(NV_MEM_CONFIG);
    _nvMemory.putString("updateToVer", _updateToVersion.getVersionDottedString());
    _nvMemory.end();

    /* Start system update task */
    xTaskCreate(
        _dispatchUpdateSystem,                                  	            //Dispatcher or task to call
        "UpdateSystem",                                                         //Task name (max 16 characters)
        8192,                                                                   //Stack size
        this,                                                                   //Pointer to this class to use as parameter (for casting later on)
        1,                                                                      //Priority
        &_taskHandlerUpdateSystem                                               //With task handle we will be able to manipulate with this task.
    );
}
#pragma endregion

#pragma region Getters
/******************************************************************************/
/*!
  @brief    Returns the state.
  @returns  uint8_t             Current state
*/
/******************************************************************************/
uint8_t UpdateManager::getState() {
    return _state;
}
#pragma endregion

#pragma region Utilities
/******************************************************************************/
/*!
  @brief    Sends the update state to the master controller.
*/
/******************************************************************************/
void UpdateManager::_sendState() {
    HTTPClient http;
    int16_t responseCode;

    String url = String(MASTER_SERVER_ADDRESS) + MASTER_CMD_SET_OTA_STATE;
    
    String payload = String("state=") + String(_state);
    payload += "&id=" + String(_ledstripId);
    
    http.begin(url);
    http.setConnectTimeout(HTTP_TIMEOUT);
    responseCode = http.POST(payload);
    http.end();                                                                 //Free resources

    if (responseCode != HTTP_CODE_OK) {
        _l.logw("Cannot send state to master controller, code: " + String(responseCode));
    }
}

/******************************************************************************/
/*!
  @brief    Casts the class to an object to be used in the task and dispatches
            the update task.
  @param    parameter           The pointer to the class running the task
                                function
*/
/******************************************************************************/
void UpdateManager::_dispatchUpdateSystem(void* parameter) {
    UpdateManager* staticObjectCast = static_cast<UpdateManager *>(parameter);
    staticObjectCast->_taskUpdateSystem();
}

/******************************************************************************/
/*!
  @brief    Task. Installs the firmware file.
*/
/******************************************************************************/
void UpdateManager::_taskUpdateSystem() {
    _l.logi("Downloading firmware file with version [" + _updateToVersion.getVersionDottedString() + "]");
    bool downloadSuccess = false;
    uint8_t retry = 0;


    while (!downloadSuccess && retry < DOWNLOAD_RETRIES) {
        if (_downloadOtaFile()) {
            downloadSuccess = true;
        }
    }

    /* If download failed, abort */
    if (!downloadSuccess) {
        _state = OTA_STATE_IDLE;
        //_state = OTA_STATE_FAILED;//TODO implement
        _powerCycles = 0;

        /* Save state in non-volatile memory */
        _nvMemory.begin(NV_MEM_CONFIG);
        _nvMemory.putUShort("state", _state);
        _nvMemory.putUChar("powerCycles", _powerCycles);
        _nvMemory.end();

        _taskHandlerUpdateSystem = NULL;
        vTaskDelete(_taskHandlerUpdateSystem);
        _sendState();
    }

    _l.logi("Files downloaded, installing firmware", false);

    _installOtaFile();
    
    _taskHandlerUpdateSystem = NULL;
    vTaskDelete(_taskHandlerUpdateSystem);
}

/******************************************************************************/
/*!
  @brief    Downloads the specified OTA file type.
  @param    type                0 = firmware, 1 = filesystem
  @returns  bool                True if successful
*/
/******************************************************************************/
bool UpdateManager::_downloadOtaFile() {
    _state = OTA_STATE_DOWNLOADING_FIRMWARE;
    String filename = _updateToVersion.getVersionDottedString() + ".bin";
    String path = _memoryManager.joinPaths(OTA_FIRMWARE_DIRECTORY, filename);

    /* Download and write to file */
    uint8_t* fileBufferPointer = (uint8_t*) malloc(FILE_CHUNK_SIZE);
    uint8_t* downloadedBufferPointer = fileBufferPointer;
    uint8_t retry = 0;

    _sendState();

    String url = String(MASTER_SERVER_ADDRESS) + MASTER_CMD_DOWNLOAD_OTA_FILE;
    url += "?version=" + _updateToVersion.getVersionDottedString();

    /* Create HTTPS client for HTTPS download */
    WiFiClientSecure *client = new WiFiClientSecure;
    if(!client) {
        _l.logd("Fail");
    }
    client->setInsecure();

    HTTPClient http;
    http.useHTTP10(true);                                                       //Prevent chunked encoding

    /* Connect to download server */
    while (!http.begin(*client, url)) {
        retry++;

        /* If still has retries left */
        if (retry <= DOWNLOAD_RETRIES) {
            vTaskDelay(500);                                                    //Wait some time to possibly restore the connection
            continue;                                                           //Try again
        } else {
            _l.loge("Download could not be started, could not connect to server");
            http.end();
            free(fileBufferPointer);                                            //Free memory resources
            return false;
        }
    }

    http.setConnectTimeout(CONNECTION_TIMEOUT);
    int16_t responseCode = http.GET();                                          //Send HTTP request to gondola

    while (responseCode != HTTP_CODE_OK) {
        /* If connection is refused, can be connection error */
        if (responseCode == HTTPC_ERROR_CONNECTION_REFUSED || responseCode == HTTPC_ERROR_READ_TIMEOUT) {
            retry++;
            
            /* If still has retries left */
            if (retry <= DOWNLOAD_RETRIES) {
                vTaskDelay(500);                                                //Wait some time to possibly restore the connection
                continue;                                                       //Try again
            }
        }
        
        _l.loge("Download could not be started [" + http.errorToString(responseCode) + "], code [" + String(responseCode) + "]");
        http.end();
        free(fileBufferPointer);                                                //Free memory resources
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    stream->setTimeout(3);
    
    while (!stream->available()) {
        _l.loge("Download stream timeout");
        retry++;

        /* If still has retries left */
        if (retry <= DOWNLOAD_RETRIES) {
            vTaskDelay(5);                                                      //Give download some time
            continue;                                                           //Try again
        } else {
            http.end();
            free(fileBufferPointer);                                            //Free memory resources
            return false;
        }
    }

    /* Create new file */
    File file = SD_MMC.open(path, "w");

    if (!file) {
        _l.loge("Cannot create new file");
        http.end();
        return false;
    }
    
    /* Download and write to file */
    size_t fileSize = http.getSize();
    if (fileSize <= 0) {
        _l.logw("Unknown content length (chunked transfer)");
    }
    size_t remainingBytes = fileSize;

    _l.logi("Started download, file [" + path + "], size [" + String(fileSize/BYTES_IN_KBYTE) + "kB]");

    uint32_t startTime = millis();
    bool timeout = false;
    uint16_t timeoutCounter = 0;
    int32_t downloadedBytes = 0;
    int32_t previousDataSize = 0;

    while (remainingBytes > 0 && http.connected() && !timeout) {
        previousDataSize = downloadedBytes;
        downloadedBytes = stream->available();

        /* If downloaded size is 0, start timeout timer */
        if (downloadedBytes <= 0) {
            timeoutCounter++;
            if (timeoutCounter > 500) {
                _l.logd("OTA download timed out");
                timeout = true;
                break;
            }
            
            vTaskDelay(5);                                                      //Give download some time
            continue;
        } else if (downloadedBytes == previousDataSize) {
            timeoutCounter++;
            if (timeoutCounter > 500) {
                _l.logd("OTA download stall detected");
                timeout = true;
                break;                                                          //Break out of download loop
            }

            if (timeoutCounter % 100 == 0) {
                /* Try to read a byte from stream (to solve frozen state)*/
                uint8_t singleByte;
                if (stream->read(&singleByte, 1) == 1) {
                    remainingBytes--;                                           //One byte less remaining
                    downloadedBufferPointer++;                                  //One byte already downloaded
                }
            }

            vTaskDelay(5);                                                      //Give download some time
            continue;
        } else {
            timeoutCounter = 0;
        }

        int32_t availableBytes = FILE_CHUNK_SIZE - (downloadedBufferPointer - fileBufferPointer);   //Calculate available bytes to download for chunk
        int32_t bytesToRead = std::min(downloadedBytes, availableBytes);        //If downloaded size is bigger than available bytes in write buffer, only read available bytes
        int32_t readBytes = stream->read(downloadedBufferPointer, bytesToRead); //Read bytes from download stream

        downloadedBufferPointer += readBytes;                                   //Update pointer address
        remainingBytes -= readBytes;                                            //Update remaining bytes

        /* If the last bytes are downloaded, no bytes are available to fill chunk */
        if (remainingBytes == 0) {
            availableBytes = 0;
        } else {
            availableBytes -= readBytes;
        }

        /* If buffer is filled, write to SD card */
        if (availableBytes == 0) {
            size_t writeSize = downloadedBufferPointer - fileBufferPointer;

            if (file.write(fileBufferPointer, writeSize) == 0) {
                /* Reopen file in append mode */
                file.close();
                file = SD_MMC.open(path, "a");

                if (!file) {
                    timeout = true;
                    break;
                }

                /* Second try to write */
                if (file.write(fileBufferPointer, writeSize) == 0) {
                    _l.loge("Failed to write chunk to file");
                    timeout = true;
                    break;
                }
                
                timeoutCounter = 0;                                             //Reset timeout counter
            }

            downloadedBufferPointer = fileBufferPointer;
        }
    }
    uint32_t endTime = millis();
    file.close();
    http.end();
    free(fileBufferPointer);                                                    //Free memory resources

    if (timeout) {
        SD_MMC.remove(path.c_str());                                            //Delete failed file
        return false;
    }
    
    /* Check file size */
    struct stat fileInfo;
    String fullPath = String(SD_MOUNTPOINT) + path;
    stat(fullPath.c_str(), &fileInfo);

    if (remainingBytes == 0 && fileInfo.st_size == fileSize) {
        _l.logi("Download success", false);
    } else {
        _l.loge("Download failed");
        SD_MMC.remove(path.c_str());                                            //Delete failed file
        return false;
    }

    uint32_t elapsedTime = (endTime - startTime) / 1000;

    if (elapsedTime != 0) {
        uint32_t downloadSpeed = fileSize / elapsedTime;
        _l.logi("Average speed [" + String(downloadSpeed/BYTES_IN_KBYTE) + "kB/sec]");
    }

    return true;
}

/******************************************************************************/
/*!
  @brief    Installs the update file on the controller.
*/
/******************************************************************************/
void UpdateManager::_installOtaFile() {
    String filename;
    String path;

    _state = OTA_STATE_INSTALLING_FIRMWARE;
    filename = _updateToVersion.getVersionDottedString() + ".bin";
    path = _memoryManager.joinPaths(OTA_FIRMWARE_DIRECTORY, filename);


    _l.logi("Installing [" + filename + "]");

    File file = SD_MMC.open(path);
    if (!file) {
        _l.loge("File [" + path + "] not found");
        return;
    }
    
    Update.begin(file.size(), U_FLASH);

    Update.writeStream(file);

    if (Update.end()) {
        _l.logi("Update successfully to version [" + _updateToVersion.getVersionDottedString() + "], restarting now");
    } else {
        _l.loge("Cannot update to version [" + _updateToVersion.getVersionDottedString() + "], " + Update.errorString());
    }

    file.close();

    if (!SD_MMC.remove(path)) {
        _l.loge("Update file could not be deleted");
    }

    _state = OTA_STATE_FINISHED;

    /* Save state in non-volatile memory */
    _nvMemory.begin(NV_MEM_CONFIG);
    _nvMemory.putUShort("state", _state);
    _nvMemory.end();

    ESP.restart();
}
#pragma endregion