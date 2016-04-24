/**
 * The MIT License (MIT)
 * Copyright (c) 2016 Ethan Gaebel <egaebel@vt.edu>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

// g++ -std=c++11 -g3 -ggdb -O0 -I . lgtm_crypto.cpp lgtm_crypto_runner.cpp ../../cryptopp/libcryptopp.a -o lgtm_crypto_runner -lcryptopp -static -lpthread

// Uncomment to enable test running
// #define TESTING

#include "lgtm_crypto_runner.hpp"

//~Constants----------------------------------------------------------------------------------------
static const string LGTM_CRYPTO_PREFIX = ".lgtm-crypto-params-";
// Crypto parameters file names
static const string PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "public-key";
static const string PRIVATE_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "private-key";
static const string SHARED_SECRET_FILE_NAME = LGTM_CRYPTO_PREFIX + "shared-secret";
static const string COMPUTED_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "computed-key";
static const string CURRENT_IV = LGTM_CRYPTO_PREFIX + "initialization-vector";
// "Other" crypto parameters file names
static const string OTHER_PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-public-key";

// Random numbers
static const string FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "first-message-random-number";
static const string OTHER_FIRST_MESSAGE_RANDOM_NUMBER = LGTM_CRYPTO_PREFIX 
        + "other-first-message-random-number";

// Facial recognition params files
static const string FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-facial-recognition-params";

// Received facial recognition params files
static const string RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-received-facial-recognition-params";

// Message file names
static const string FIRST_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message";
static const string FIRST_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message-reply";

static const string THIRD_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message";
static const string THIRD_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message-reply";

// Crypto parameters
static const unsigned int RANDOM_NUMBER_SIZE = 256;

//~Functions----------------------------------------------------------------------------------------
/**
 * Create Diffie-Hellman parameters and save them to files.
 */
void firstMessage() {
    cout << "First Message" << endl;
    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);

    // Write public key and private key to files
    writeToFile(PRIVATE_KEY_FILE_NAME, privateKey);
    writeToFile(PUBLIC_KEY_FILE_NAME, publicKey);

    // Generate random number
    SecByteBlock firstMessageRandomNumber;
    generateRandomNumber(firstMessageRandomNumber, RANDOM_NUMBER_SIZE);

    // Write random number to file
    writeToFile(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME, firstMessageRandomNumber);

    // Combine public key and random number into one file
    vector<string> inputFiles;
    inputFiles.push_back(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    inputFiles.push_back(PUBLIC_KEY_FILE_NAME);
    combineFiles(inputFiles, FIRST_MESSAGE_FILE_NAME);
}

/**
 * Create Diffie-Hellman parameters and save them to files.
 * Compute the shared secret and computed key from the received other public key.
 * Save shared secret and computed key to file.
 */
bool replyToFirstMessage() {
    cout << "Reply To First Message" << endl;

    // Split received file into separate files
    vector<string> outputFiles;
    outputFiles.push_back(OTHER_FIRST_MESSAGE_RANDOM_NUMBER);
    outputFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    vector<int> bytesPerFile;
    bytesPerFile.push_back(RANDOM_NUMBER_SIZE);
    splitFile(FIRST_MESSAGE_FILE_NAME, outputFiles, bytesPerFile);

    // Read in received Diffie-Hellman public key from file
    SecByteBlock otherPublicKey;
    readFromFile(OTHER_PUBLIC_KEY_FILE_NAME, otherPublicKey);

    // Read in received random number from file
    SecByteBlock otherFirstMessageRandomNumber;
    readFromFile(OTHER_FIRST_MESSAGE_RANDOM_NUMBER, otherFirstMessageRandomNumber);

    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);

    // Write public key and private key to files
    writeToFile(PRIVATE_KEY_FILE_NAME, privateKey);
    writeToFile(PUBLIC_KEY_FILE_NAME, publicKey);

    // Compute shared secret
    SecByteBlock sharedSecret;
    if (!diffieHellmanSharedSecretAgreement(sharedSecret, otherPublicKey, privateKey)) {
        cerr << "Security Error in replyToFirstMessage. Diffie-Hellman shared secret could not be agreed to." << endl;
        return false;
    }

    // Compute key from shared secret
    SecByteBlock key;
    generateSymmetricKeyFromSharedSecret(key, sharedSecret);

    // Write shared secret and computed key to file
    writeToFile(SHARED_SECRET_FILE_NAME, sharedSecret);
    writeToFile(COMPUTED_KEY_FILE_NAME, key);

    // Generate random number
    SecByteBlock firstMessageRandomNumber;
    generateRandomNumber(firstMessageRandomNumber, RANDOM_NUMBER_SIZE);

    // Write random number to file
    writeToFile(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME, firstMessageRandomNumber);

    vector<string> inputFileNames;
    inputFileNames.push_back(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    inputFileNames.push_back(PUBLIC_KEY_FILE_NAME);
    combineFiles(inputFileNames, FIRST_MESSAGE_REPLY_FILE_NAME);

    return true;
}

/**
 * Process public key and random number received from other user.
 * Encrypt facial recognition parameters.
 */
bool thirdMessage() {
    cout << "Third Message" << endl;

    // Split received file into random number and public key
    vector<string> outputFiles;
    outputFiles.push_back(OTHER_FIRST_MESSAGE_RANDOM_NUMBER);
    outputFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    vector<int> bytesPerFile;
    bytesPerFile.push_back(RANDOM_NUMBER_SIZE);
    splitFile(FIRST_MESSAGE_REPLY_FILE_NAME, outputFiles, bytesPerFile);

    // Read from file
    SecByteBlock privateKey;
    SecByteBlock otherPublicKey;
    readFromFile(PRIVATE_KEY_FILE_NAME, privateKey);
    readFromFile(OTHER_PUBLIC_KEY_FILE_NAME, otherPublicKey);

    // Compute shared secret
    SecByteBlock sharedSecret;
    if (!diffieHellmanSharedSecretAgreement(sharedSecret, otherPublicKey, privateKey)) {
        cerr << "Security Error in thirdMessage. Diffie-Hellman shared secret could not be agreed to." << endl;
        return false;
    }

    // Compute key from shared secret
    SecByteBlock key;
    generateSymmetricKeyFromSharedSecret(key, sharedSecret);

    // Write shared secret and computed key to file
    writeToFile(SHARED_SECRET_FILE_NAME, sharedSecret);
    writeToFile(COMPUTED_KEY_FILE_NAME, key);

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Encrypt facial recognition params
    encryptFile(FACIAL_RECOGNITION_FILE_NAME, 
            THIRD_MESSAGE_FILE_NAME, 
            key, curIv);
    return true;
}

/**
 * Decrypt received facial recognition parameters.
 * Encrypt facial recognition parameters.
 */
bool replyToThirdMessage() {
    cout << "Reply To Third Message" << endl;
    // Read session key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Decrypt received facial recognition params
    if (!decryptFile(THIRD_MESSAGE_FILE_NAME, 
            RECEIVED_FACIAL_RECOGNITION_FILE_NAME,
            key, curIv)) {
        cerr << "Security Error in replyToThirdMessage. MAC could not be verified." << endl;
        return false;
    }

    // Encrypt facial recognition params
    encryptFile(FACIAL_RECOGNITION_FILE_NAME, 
            THIRD_MESSAGE_REPLY_FILE_NAME, 
            key, curIv);
    return true;
}

/**
 * Decrypt and verify the third message reply.
 */
bool decryptThirdMessageReply() {
    cout << "Decrypt Third Message Reply" << endl;
    // Read session key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Decrypt facial recognition params
    if (!decryptFile(THIRD_MESSAGE_REPLY_FILE_NAME,
            RECEIVED_FACIAL_RECOGNITION_FILE_NAME,
            key, curIv)) {
        cerr << "Security Error in decryptThirdMessageReply. MAC could not be verified." << endl;
        return false;
    }

    return true;
}

//~Main function------------------------------------------------------------------------------------
#ifndef TESTING
int main(int argc, char *argv[]) {

    if (argc != 2) {
        cout << "Incorrect number of aruments passed to lgtm_crypto_runner.cpp. Expected 2 but got: " << argc << endl;
        return 1; 
    }

    if (strncmp(argv[1], "first-message-reply", 19) == 0) {
cout << "first-message-reply" << endl;
        if (!replyToFirstMessage()) {
            return 1;
        }
    } else if (strncmp(argv[1], "first-message", 13) == 0) {
cout << "first-message" << endl;
        firstMessage();
    } else if (strncmp(argv[1], "third-message-reply", 19) == 0) {
cout << "third-message-reply" << endl;
        if (!replyToThirdMessage()) {
            return 1;
        }
    } else if (strncmp(argv[1], "third-message", 13) == 0) {
cout << "third-message" << endl;
        if (!thirdMessage()) {
            return 1;
        }
    } else if (strncmp(argv[1], "decrypt-third-message-reply", 27) == 0) {
cout << "decrypt-third-message-reply" << endl;
        if (!decryptThirdMessageReply()) {
            return 1;
        }
    } else {
        cout << "Argument:" << argv[1] << " was not recognized in lgtm_crypto_runner.cpp! Exiting...." << endl;
    }
    return 0;
}
#endif
