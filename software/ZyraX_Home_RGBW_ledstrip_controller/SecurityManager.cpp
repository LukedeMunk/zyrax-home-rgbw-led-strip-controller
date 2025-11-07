/******************************************************************************/
/*
 * File:    SecurityManager.cpp
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
#include "SecurityManager.h"

/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
SecurityManager::SecurityManager() {
    _l.setTag("SecurityManager");
}

/******************************************************************************/
/*!
  @brief    Hashes (SHA) the specified string with a salt and returns it.
  @param    string              String to hash
  @returns  String              Hashed string
*/
/******************************************************************************/
String SecurityManager::hash(const char* string) {
    byte hashedValue[HASH_LENGTH];
    byte saltedHashedValue[HASH_LENGTH*2];
    String hashString;

    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    const size_t payloadLength = strlen(string);         

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *) string, payloadLength);
    mbedtls_md_finish(&ctx, hashedValue);
    mbedtls_md_free(&ctx);

    /* Add salt */
    for (uint8_t i = 0; i < sizeof(hashedValue); i++) {
        saltedHashedValue[i*2] = hashedValue[i];                                //Actual hash
        if (i < sizeof(hashedValue) - 1) {
            saltedHashedValue[i*2+1] = (byte) esp_random();                     //Random salt
        }
    }

    /* Convert to String */
    for (uint8_t i = 0; i < sizeof(saltedHashedValue); i++) {
        char str[3];
        sprintf(str, "%02x", (int) saltedHashedValue[i]);
        hashString += str;
    }

    return hashString;
}

/******************************************************************************/
/*!
  @brief    Compares the specified hash string and plain string.
  @param    hashedString        Hashed string
  @param    plainString         Plain string
  @returns  bool                True if strings are equal
*/
/******************************************************************************/
bool SecurityManager::compareHashString(String hashedString, const char* plainString) {
    String hashedString2 = hash(plainString);
    uint8_t byteCounter = 0;                                                    //To ignore salt bytes

    /* Compare the strings, ignoring salt bytes */
    for (uint8_t i = 0; i < sizeof(hashedString); i++) {
        /* If a non-salted byte differs, strings differ */
        if (byteCounter < 2 && hashedString[i] != hashedString2[i]) {
            return false;
        }
        byteCounter++;
        byteCounter %= 4;
    }

    return true;
}

/******************************************************************************/
/*!
  @brief    Encrypts (AES256) the specified string and returns it.
  @param    string              String to encrypt
  @returns  String              Encrypted string
*/
/******************************************************************************/
String SecurityManager::encrypt(const char* string) {
    unsigned char encryptedValue[ENCRYPTION_LENGTH] = {0};
    String encryptedString;

    /* Make sure the input has the right padding (multiply of 16) */
    unsigned char input[ENCRYPTION_LENGTH] = {0};
    sprintf((char*)input, "%s", string);
    
    mbedtls_aes_context aes;
                                        
    /* Saved without encryption, so not secured against firmware read-outs */
    /* TODO_IN_PRODUCTION: change values */
    unsigned char initializationVector[16] = {
                                            0x00, 0x01, 0x02, 0x03,
                                            0x04, 0x05, 0x06, 0x07,
                                            0x08, 0x09, 0x0A, 0x0B,
                                            0x0C, 0x0D, 0x0E, 0x0F
                                            };

    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, _aesKey, AES256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, ENCRYPTION_LENGTH, initializationVector, input, encryptedValue);
    mbedtls_aes_free(&aes);

    /* Convert to String */
    for (uint8_t i = 0; i < sizeof(encryptedValue); i++) {
        char str[3];
        sprintf(str, "%02x", (int) encryptedValue[i]);
        encryptedString += str;
    }

    return encryptedString;
}

/******************************************************************************/
/*!
  @brief    Decrypts (AES256) the specified string and returns it.
  @param    string              String to decrypt
  @returns  String              Decrypted string
*/
/******************************************************************************/
String SecurityManager::decrypt(const char* string) {
    if (string == "") {
        return "";
    }
                                            
    unsigned char encryptedValue[ENCRYPTION_LENGTH] = {0};
    unsigned char decryptedValue[ENCRYPTION_LENGTH] = {0};
    String decryptedString;
    
    /* Convert to unsigned char */
    for (uint8_t i = 0; i < strlen(string); i+=2) {
        char str[3];
        str[0] = string[i];
        str[1] = string[i+1];
        *(encryptedValue + (i >> 1)) = strtoul(str, NULL, 16);
    }
    
    mbedtls_aes_context aes;
                                        
    /* Saved without encryption, so not secured against firmware read-outs */
    /* TODO_IN_PRODUCTION: change values */
    unsigned char initializationVector[16] = {
                                            0x00, 0x01, 0x02, 0x03,
                                            0x04, 0x05, 0x06, 0x07,
                                            0x08, 0x09, 0x0A, 0x0B,
                                            0x0C, 0x0D, 0x0E, 0x0F
                                            };

    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, _aesKey, AES256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ENCRYPTION_LENGTH, initializationVector, encryptedValue, decryptedValue);
    mbedtls_aes_free(&aes);

    /* Convert to String */
    for (uint8_t i = 0; i < sizeof(decryptedValue); i++) {
        char str[3];
        sprintf(str, "%c", (char) decryptedValue[i]);
        decryptedString += str;
    }

    return decryptedString;
}