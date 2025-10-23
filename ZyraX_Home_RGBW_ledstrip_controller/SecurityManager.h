/******************************************************************************/
/*
 * File:    SecurityManager.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   SecurityManager class for handling encrypting, decrypting, hashing
 *          and verifying text variables.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H
#include "Arduino.h"                                                            //For additional Arduino framework features, like String object
#include "stdint.h"                                                             //For size defined int types
#include "mbedtls/md.h"                                                         //For SHA hashing
#include "mbedtls/aes.h"                                                        //For AES encryption
#include "esp_random.h"                                                         //For random salt bytes

#include "Logger.h"                                                             //For printing and saving logs
#include "Configuration.h"                                                      //For configuration variables and global constants

#define AES256                          256
#define AES256_BYTES                    32

#define HASH_LENGTH                     32
#define ENCRYPTION_LENGTH               64

class SecurityManager {
    public:
        SecurityManager();

        String hash(const char* string);
        bool compareHashString(String hashedString, const char* plainString);
        
        String encrypt(const char* string);
        String decrypt(const char* string);

    private:
        Logger _l;

        /* Saved without encryption, so not secured against firmware read-outs */
        /* TODO_IN_PRODUCTION: change values */
        unsigned char _aesKey[AES256_BYTES] = {
                                            0x00, 0x01, 0x02, 0x03, 
                                            0x04, 0x05, 0x06, 0x07,
                                            0x08, 0x09, 0x0A, 0x0B,
                                            0x0C, 0x0D, 0x0E, 0x0F,
                                            0x10, 0x11, 0x12, 0x13,
                                            0x14, 0x15, 0x16, 0x17,
                                            0x18, 0x19, 0x1A, 0x1B,
                                            0x1C, 0x1D, 0x1E, 0x1F
                                        };
};
#endif