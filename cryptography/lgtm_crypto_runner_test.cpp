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

// g++ -std=c++11 -g3 -ggdb -O0 -DDEBUG lgtm_crypto.cpp lgtm_crypto_runner.cpp lgtm_crypto_runner_test.cpp -o lgtm_crypto_runner_test -lcryptopp -lpthread

#include "lgtm_crypto.hpp"
#include "lgtm_crypto_runner.hpp"

#include "../../cryptopp/filters.h"
#include "../../cryptopp/secblock.h"

#include <fstream>
#include <string>

using CryptoPP::SecByteBlock;

using std::ios;
using std::ofstream;
using std::string;

//~Global variables---------------------------------------------------------------------------------
// General constants
static const string RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING = "RECEIVED-FACIAL-RECOGNITION-PARAMS";

// Common prefix
static const string LGTM_CRYPTO_PREFIX = ".lgtm-crypto-params-";

// Crypto params
static const string PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "public-key";
static const string VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "verification-hash";
static const string FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "facial-recognition-verification-hash";

// "Other" Crypto params
static const string OTHER_PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-public-key";
static const string COMPUTED_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "computed-key";
static const string OTHER_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-verification-hash";
static const string OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "facial-recognition-other-verification-hash";

// Facial recognition files
static const string FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params";
static const string ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params--encrypted";
static const string RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-received-facial-recognition-params";
static const string DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-received-facial-recognition-params--decrypted";

// Test Files
static const string TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-test-unencrypted-received-facial-recognition-params";

//~File IO Functions--------------------------------------------------------------------------------
/**
 * Reads into a SecByteBlock from a file specified by fileName.
 */
static void readFromFile(const string &fileName, SecByteBlock &input) {
    cout << "TEST read from file, reading " << input.SizeInBytes() 
            << " bytes from file: " << fileName << endl;
    ifstream inputStream(fileName, ios::in | ios::binary);
    // Get file length
    inputStream.seekg(0, inputStream.end);
    int fileLength = inputStream.tellg();
    inputStream.seekg(0, inputStream.beg);
    input.CleanNew(fileLength);
    inputStream.read((char*) input.BytePtr(), input.SizeInBytes());
    inputStream.close();
}

/**
 * Writes a SecByteBlock to a file specified by fileName.
 */
static void writeToFile(const string &fileName, SecByteBlock &output) {
    cout << "TEST write to file, writing " << output.SizeInBytes() 
            << " bytes to file: " << fileName << endl;
    ofstream outputStream(fileName, ios::out | ios::binary);
    outputStream.write((char*) output.BytePtr(), output.SizeInBytes());
    outputStream.close();
}

//~Simulated Reply Functions------------------------------------------------------------------------
/**
 * Simulate a reply to the first message by creating a file holding another public key.
 */
void simulateFirstMessageReply() {
    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);
    // Write other public key to file
    writeToFile(OTHER_PUBLIC_KEY_FILE_NAME, publicKey);
}

/**
 * Simulate a reply to the second message with a verification hash.
 */
void simulateOneWayFirstVerification() {
    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    createHashFromFiles(hashFiles, OTHER_VERIFICATION_HASH_FILE_NAME);
}

/**
 * Simulate a reply to the second message with a verification hash.
 */
void simulateOtherWayFirstVerification() {
    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    createHashFromFiles(hashFiles, OTHER_VERIFICATION_HASH_FILE_NAME);
}

/**
 * Simulate a reply to the third message by creating a dummy file for received, encrypted 
 * facial recognition parameters.
 */
void simulateThirdMessageReply() {
    SecByteBlock symmetricKey;
    readFromFile(COMPUTED_KEY_FILE_NAME, symmetricKey);

    // Write test string to unencrypted received facial recognition params
    ofstream plainTextOutputStream(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            ios::out | ios::binary);
    plainTextOutputStream.write(RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.data(), 
            RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.length());
    plainTextOutputStream.close();

    // Encrypt received facial recognition params
    // TODO: This test will need to get more sophisticated when the IV is set differently.
    byte iv[AES::BLOCKSIZE];
    memset(iv, 0, AES::BLOCKSIZE);
    encryptFile(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, "", 
            RECEIVED_FACIAL_RECOGNITION_FILE_NAME, symmetricKey, iv);
}

/**
 *
 */
void simulateFacialOneWayVerification() {
    // Generate simulated hash verification code for "other" sender
    vector<string> hashFiles;
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    createHashFromFiles(hashFiles, OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
}

/**
 *
 */
void simulateFacialOtherWayVerification() {
    // Generate hash for other person's verification
    vector<string> hashFiles;
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    createHashFromFiles(hashFiles, OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
}

//~Corruption Testing Function----------------------------------------------------------------------
/**
 * Corrupts three bytes of the "received" facial recognition file.
 */
void corruptThirdMessageReply() {
    byte corruption[] = {0x4, 0x4, 0x4};
    ofstream outputStream(RECEIVED_FACIAL_RECOGNITION_FILE_NAME, ios::in | ios::out | ios::binary);
    outputStream.seekp(4);
    outputStream.write((char*) corruption, 3); 
    outputStream.close();
}

//~Final Testing Check Function---------------------------------------------------------------------
/**
 * Check if the string in the decrypted facial recognition file matches the string written to it.
 */
bool checkFacialRecognitionFile() {
    ifstream inputStream(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, ios::in);
    if (!inputStream.is_open()) {
        cout << "Failure in checkFacialRecognitionFile: " 
                << DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME << " could not be opened...." 
                << endl;
        return false;
    }
    inputStream.seekg(0, inputStream.end);
    int fileLength = inputStream.tellg();
    inputStream.seekg(0, inputStream.beg);
    if (fileLength < 1) {
        cout << "Failure in checkFacialRecognitionFile: " 
                << DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
                << " has invalid fileLength of" << fileLength
                << endl;
        return false;
    }
    char* facialRecognitionString = new char[fileLength];
    inputStream.read(facialRecognitionString, fileLength);
    inputStream.close();

    bool equalityCheck = (RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.compare(
            string(facialRecognitionString)) == 0);

    delete[] facialRecognitionString;

    return equalityCheck;
}

//~Each Type of Testing Function--------------------------------------------------------------------
/**
 * Run a full test from the first sender's point of view.
 */ 
bool testOneWay() {
    cout << endl << "testOneWay: " << endl;
    // Generate and save Diffie-Hellman parameters
    firstMessage();
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessageReply();
    // Generate symmetric key and save
    secondMessage();
    // Simulate reception of a hash for verification
    simulateOneWayFirstVerification();
    // Encrypt facial recognition file
    thirdMessage();
    // Simulate reception of facial recognition params (just a dummy file)
    simulateThirdMessageReply();
    //
    simulateFacialOneWayVerification();
    // Decrypt and verify the third message
    decryptThirdMessageReply();
    if(checkFacialRecognitionFile()) {
        cout << endl << "Test One Way Passed!" << endl << endl;
        return true;
    } else {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
}

bool testOneWayCorruption() {
    cout << endl << "testOneWayCorruption: " << endl;
    // Generate and save Diffie-Hellman parameters
    firstMessage();
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessageReply();
    // Generate symmetric key and save
    secondMessage();
    // Simulate reception of a hash for verification
    simulateOneWayFirstVerification();
    // Encrypt facial recognition file
    thirdMessage();
    // Simulate reception of facial recognition params (just a dummy file)
    simulateThirdMessageReply();
    // Corrupt the third message reply!!!!
    corruptThirdMessageReply();
    //
    simulateFacialOneWayVerification();
    // Catch the error that should be thrown due to the corruption above
    try {
        // Decrypt and verify the third message
        decryptThirdMessageReply();   
        cout << endl << "One Way Corruption Test FAILED!!!!!!" << endl << endl;
        return false;
    } catch (const CryptoPP::HashVerificationFilter::HashVerificationFailed &e) {
        cout << endl << "One Way Corruption Test Passed!" << endl << endl;
        return true;
    }
}

/**
 * Run a full test from the second sender's point of view.
 */
bool testOtherWay() {
    cout << endl << "testOtherWay: " << endl;
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessageReply();
    // Generate and save Diffie-Hellman parameters
    replyToFirstMessage();
    // Simulate reception of a hash for verification
    simulateOtherWayFirstVerification();
    // Perform verification on received hash
    replyToSecondMessage();
    //
    simulateFacialOtherWayVerification();
    // Simulate reception of facial recognition params (just a dummy file)
    simulateThirdMessageReply();
    // Decrypt and verify the third message
    replyToThirdMessage();
    if(checkFacialRecognitionFile()) {
        cout << endl << "Test Other Way Passed!" << endl << endl;
        return true;
    } else {
        cout << endl << "Test Other Way FAILED!!!!!!" << endl << endl;
        return false;
    }
}

/**
 * Run a full test from the second sender's point of view.
 */
bool testOtherWayCorruption() {
    cout << endl << "testOtherWayCorruption: " << endl;
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessageReply();
    // Generate and save Diffie-Hellman parameters
    replyToFirstMessage();
    // Simulate reception of a hash for verification
    simulateOtherWayFirstVerification();
    // Perform verification on received hash
    replyToSecondMessage();
    // Simulate reception of facial recognition params (just a dummy file)
    simulateThirdMessageReply();
    // Corrupt the third message reply!!!!
    corruptThirdMessageReply();
    // Catch the error that should be thrown due to the corruption above
    try {
        // Decrypt and verify the third message
        replyToThirdMessage();
        cout << endl << "Other Way Corruption Test FAILED!!!!!!" << endl << endl;
        return false;
    } catch (const CryptoPP::HashVerificationFilter::HashVerificationFailed &e) {
        cout << endl << "Other Way Corruption Test Passed!" << endl << endl;
        return true;
    }
}

//~Main Runner Function-----------------------------------------------------------------------------
/**
 * Run tests.
 */
int main(int argc, char *argv[]) {

    // Run a specific test
    if (argc > 1) {
        int testNumber = atoi(argv[1]);
        switch(testNumber) {
            case 1:
            {
                testOneWay();
                return 0;
            }
            case 2:
            {
                testOneWayCorruption();
                return 0;
            }
            case 3:
            {
                testOtherWay();
                return 0;
            }
            case 4:
            {
                testOtherWayCorruption();
                return 0;
            }
            default:
            {
                cout << "Tests are numbered 1-4, please re-enter your input and try again." << endl;
                return 0;
            }
        }
    }

    // Run all tests
    if (!testOneWay()) {
        return 1;
    }
    if (!testOneWayCorruption()) {
        return 1;
    }
    if (!testOtherWay()) {
        return 1;
    }
    if (!testOtherWayCorruption()) {
        return 1;
    }
    return 0;
}

/*
if (strncmp(argv[0], "first-message", 13)) {
    firstMessage();
} else if (strncmp(argv[0], "first-message-reply", 19)) {
    replyToFirstMessage();
} else if (strncmp(argv[0], "second-message", 14)) {
    secondMessage();
} else if (strncmp(argv[0], "second-message-reply", 20)) {
    replyToSecondMessage();
} else if (strncmp(argv[0], "third-message", 13)) {
    thirdMessage();
} else if (strncmp(argv[0], "third-message-reply", 19)) {
    replyToThirdMessage();
} else if (strncmp(argv[0], "decrypt-third-message-reply", 27)) {
    decryptThirdMessageReply();
}
*/