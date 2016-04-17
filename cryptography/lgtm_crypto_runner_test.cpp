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
#include "lgtm_file_utils.hpp"

#include "../../cryptopp/filters.h"
#include "../../cryptopp/secblock.h"

#include <fstream>
#include <string>
#include <vector>

using CryptoPP::SecByteBlock;

using std::ios;
using std::ofstream;
using std::string;
using std::vector;

//~Global variables---------------------------------------------------------------------------------
// Common prefix
static const string LGTM_CRYPTO_PREFIX = ".lgtm-crypto-params-";
// Crypto params
static const string PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "public-key";
static const string COMPUTED_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "computed-key";    
// "Other" Crypto params
static const string OTHER_PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "other-public-key";

// Random numbers
static const string FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "first-message-random-number";
static const string OTHER_FIRST_MESSAGE_RANDOM_NUMBER = LGTM_CRYPTO_PREFIX 
        + "other-first-message-random-number";
static const string SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "second-message-random-number";
static const string OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "other-second-message-random-number";

// Hash verification files
// First verification files
static const string VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "verification-hash";
static const string ENCRYPTED_OTHER_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "encrypted-other-verification-hash";

static const string OTHER_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "other-verification-hash";

// Second, facial recognition params included, verification hashes
static const string FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "facial-recognition-verification-hash";
static const string OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "facial-recognition-other-verification-hash";

// Facial recognition params files
static const string FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-facial-recognition-params";
static const string VERIFIED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-facial-recognition-params-with-hash";
static const string ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-facial-recognition-params--encrypted";

// Received facial recognition params files
static const string RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-received-facial-recognition-params";
static const string DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-received-facial-recognition-params--decrypted";

// Message file names
static const string FIRST_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message";
static const string FIRST_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message-reply";

static const string SECOND_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "second-message";
static const string SECOND_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "second-message-reply";

static const string THIRD_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message";
static const string THIRD_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message-reply";

// TEST Message file names
static const string TEST_FIRST_MESSAGE_FILE_NAME = ".lgtm-test-first-message";
static const string TEST_FIRST_MESSAGE_REPLY_FILE_NAME = ".lgtm-test-first-message-reply";

static const string TEST_SECOND_MESSAGE_FILE_NAME = ".lgtm-test-second-message";
static const string TEST_SECOND_MESSAGE_REPLY_FILE_NAME = ".lgtm-test-second-message-reply";

static const string TEST_THIRD_MESSAGE_FILE_NAME = ".lgtm-test-third-message";
static const string TEST_THIRD_MESSAGE_REPLY_FILE_NAME = ".lgtm-test-third-message-reply";

// Test Files
static const string TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-test-unencrypted-received-facial-recognition-params";
static const string TEST_OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME 
        = ".lgtm-test-facial-recognition-other-verification-hash";
static const string TEST_VERIFIED_FACIAL_RECOGNITION_FILE_NAME
        = ".lgtm-test-facial-recognition-params-with-hash";
static const string TEST_OTHER_VERIFICATION_HASH_FILE_NAME
        = ".lgtm-test-other-verification-hash";
static const string TEST_OTHER_PUBLIC_KEY_FILE_NAME
        = ".lgtm-test-other-public-key";
static const string TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME
        = ".lgtm-test-other-first-message-random-number";
static const string TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME
        = ".lgtm-test-other-second-message-random-number";

// General constants
static const string RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING 
    = "RECEIVED-FACIAL-RECOGNITION-PARAMS";

static const unsigned int RANDOM_NUMBER_SIZE = 256;

//~Simulated Reply Functions------------------------------------------------------------------------
/**
 * Simulate a reply to the first message by creating a file holding another public key.
 */
void simulateFirstMessageReply() {
    cout << "Simulate First Message Reply" << endl;
    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);
    // Write other public key to file
    writeToFile(TEST_OTHER_PUBLIC_KEY_FILE_NAME, publicKey);

    // Generate random number
    SecByteBlock otherFirstMessageRandomNumber;
    generateRandomNumber(otherFirstMessageRandomNumber, RANDOM_NUMBER_SIZE);

    // Write random number to file
    writeToFile(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME, 
            otherFirstMessageRandomNumber);

    // Combine public key and random number into one file
    vector<string> inputFiles;
    inputFiles.push_back(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    inputFiles.push_back(TEST_OTHER_PUBLIC_KEY_FILE_NAME);
    combineFiles(inputFiles, FIRST_MESSAGE_REPLY_FILE_NAME);
}

void simulateFirstMessage() {
    cout << "Simulate First Message" << endl;
    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);
    // Write other public key to file
    writeToFile(TEST_OTHER_PUBLIC_KEY_FILE_NAME, publicKey);

    SecByteBlock firstMessageRandomNumber;
    generateRandomNumber(firstMessageRandomNumber, RANDOM_NUMBER_SIZE);

    // Write random number to file
    writeToFile(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME, 
            firstMessageRandomNumber);

    // Combine public key and random number into one file
    vector<string> inputFiles;
    inputFiles.push_back(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    inputFiles.push_back(TEST_OTHER_PUBLIC_KEY_FILE_NAME);
    combineFiles(inputFiles, FIRST_MESSAGE_FILE_NAME);
}

/**
 * Simulate a reply to the second message with a verification hash.
 */
void simulateSecondMessageReply() {
    cout << "Simulate Second Message Reply" << endl;

    // Generate random number to be included in body of message
    SecByteBlock secondMessageRandomNumber;
    generateRandomNumber(secondMessageRandomNumber, RANDOM_NUMBER_SIZE);
    writeToFile(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME, 
            secondMessageRandomNumber);

    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    createHashFromFiles(hashFiles, TEST_OTHER_VERIFICATION_HASH_FILE_NAME);

    // Combine random number with hash
    vector<string> inputFiles;
    inputFiles.push_back(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    inputFiles.push_back(TEST_OTHER_VERIFICATION_HASH_FILE_NAME);
    combineFiles(inputFiles, TEST_SECOND_MESSAGE_REPLY_FILE_NAME);

    // Read symmetric key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);
    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    encryptFile(TEST_SECOND_MESSAGE_REPLY_FILE_NAME, 
            ENCRYPTED_OTHER_VERIFICATION_HASH_FILE_NAME,
            key, curIv);
}

/**
 * Simulate a reply to the second message with a verification hash.
 */
void simulateSecondMessage() {
    cout << "Simulate Second Message" << endl;

    // Generate random number to be included in body of message
    SecByteBlock secondMessageRandomNumber;
    generateRandomNumber(secondMessageRandomNumber, RANDOM_NUMBER_SIZE);
    writeToFile(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME, 
            secondMessageRandomNumber);

    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    createHashFromFiles(hashFiles, TEST_OTHER_VERIFICATION_HASH_FILE_NAME);

    // Combine random number + hash into one file
    vector<string> inputFiles;
    inputFiles.push_back(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    inputFiles.push_back(TEST_OTHER_VERIFICATION_HASH_FILE_NAME);
    combineFiles(inputFiles, TEST_SECOND_MESSAGE_FILE_NAME);

    // Read symmetric key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);
    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);
    encryptFile(TEST_SECOND_MESSAGE_FILE_NAME, 
            ENCRYPTED_OTHER_VERIFICATION_HASH_FILE_NAME,
            key, curIv);
}

/**
 * Simulate a reply to the third message by creating a dummy file for received, encrypted 
 * facial recognition parameters.
 */
void simulateThirdMessageReply() {
    cout << "Simulate Third Message Reply" << endl;
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Write test string to unencrypted received facial recognition params
    ofstream plainTextOutputStream(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            ios::out | ios::binary);
    plainTextOutputStream.write(RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.data(), 
            RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.length());
    plainTextOutputStream.close();

    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    // Remember, the order isn't inverted because this test is run on the same machine....
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    createHashFromFiles(hashFiles, TEST_OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);

    // Combine facial recognition params + Hash
    vector<string> fileNames;
    fileNames.push_back(TEST_OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    fileNames.push_back(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    combineFiles(fileNames, TEST_THIRD_MESSAGE_REPLY_FILE_NAME);

    // Encrypt received facial recognition params
    // TODO: This test will need to get more sophisticated when the IV is set differently.
    byte curIv[AES::BLOCKSIZE];
    memset(curIv, 0, AES::BLOCKSIZE);
    encryptFile(TEST_THIRD_MESSAGE_REPLY_FILE_NAME, 
            RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);
}

/**
 * Simulate a reply to the third message by creating a dummy file for received, encrypted 
 * facial recognition parameters.
 */
void simulateThirdMessage() {
    cout << "Simulate Third Message" << endl;
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Write test string to unencrypted received facial recognition params
    ofstream plainTextOutputStream(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            ios::out | ios::binary);
    plainTextOutputStream.write(RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.data(), 
            RECEIVED_FACIAL_RECOGNITION_PARAMS_STRING.length());
    plainTextOutputStream.close();

    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    // Remember, the order isn't inverted because this test is run on the same machine....
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_FIRST_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_SECOND_MESSAGE_RANDOM_NUMBER_FILE_NAME);
    hashFiles.push_back(TEST_OTHER_VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    createHashFromFiles(hashFiles, TEST_OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);

    // Combine facial recognition params + Hash
    vector<string> fileNames;
    fileNames.push_back(TEST_OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    fileNames.push_back(TEST_UNENCRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    combineFiles(fileNames, TEST_VERIFIED_FACIAL_RECOGNITION_FILE_NAME);

    // Encrypt received facial recognition params
    // TODO: This test will need to get more sophisticated when the IV is set differently.
    byte curIv[AES::BLOCKSIZE];
    memset(curIv, 0, AES::BLOCKSIZE);
    encryptFile(TEST_VERIFIED_FACIAL_RECOGNITION_FILE_NAME,
            RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);
}

//~Corruption Testing Function----------------------------------------------------------------------
/**
 * Corrupts three bytes of the "received" facial recognition file.
 */
void corruptThirdMessageReply() {
    cout << "Corrupt Third Message Reply" << endl;
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
    if (!secondMessage()) {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of a hash for verification
    simulateSecondMessageReply();
    // Encrypt facial recognition file
    if (!thirdMessage()) {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of encrypted facial recognition params + hash
    simulateThirdMessageReply();
    // Decrypt and verify the third message
    if (!decryptThirdMessageReply()) {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
    if(checkFacialRecognitionFile()) {
        cout << endl << "Test One Way Passed!" << endl << endl;
        return true;
    } else {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
}

/**
 * Run a full test from the first sender's point of view where some bytes are corrupted. 
 */ 
bool testOneWayCorruption() {
    cout << endl << "testOneWayCorruption: " << endl;
    // Generate and save Diffie-Hellman parameters
    firstMessage();
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessageReply();
    // Generate symmetric key and save
   if (!secondMessage()) {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of a hash for verification
    simulateSecondMessageReply();
    // Encrypt facial recognition file
    if (!thirdMessage()) {
        cout << endl << "Test One Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of encrypted facial recognition params + hash
    simulateThirdMessageReply();
    // Corrupt the third message reply!!!!
    corruptThirdMessageReply();
    // Decrypt and verify the third message
    if (!decryptThirdMessageReply()) {
        cout << endl << "One Way Corruption Test Passed!" << endl << endl;
        return true;
    } else {
        cout << endl << "One Way Corruption Test FAILED!!!!!!" << endl << endl;
        return false;
    }
}

/**
 * Run a full test from the second sender's point of view.
 */
bool testOtherWay() {
    cout << endl << "testOtherWay: " << endl;
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessage();
    // Generate and save Diffie-Hellman parameters
    if (!replyToFirstMessage()) {
        cout << endl << "Test Other Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of a hash for verification
    simulateSecondMessage();
    // Perform verification on received hash
    if (!replyToSecondMessage()) {
        cout << endl << "Test Other Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of encrypted facial recognition params + hash
    simulateThirdMessage();
    // Decrypt and verify the third message
    if (!replyToThirdMessage()) {
        cout << endl << "Test Other Way Failed!" << endl << endl;
        return false;
    }
    if(checkFacialRecognitionFile()) {
        cout << endl << "Test Other Way Passed!" << endl << endl;
        return true;
    } else {
        cout << endl << "Test Other Way FAILED!!!!!!" << endl << endl;
        return false;
    }
}

/**
 * Run a full test from the second sender's point of view except some bytes are corrupted.
 */
bool testOtherWayCorruption() {
    cout << endl << "testOtherWayCorruption: " << endl;
    // Simulate reception of unencrypted Diffie-Hellman public key
    simulateFirstMessage();
    // Generate and save Diffie-Hellman parameters
    if (!replyToFirstMessage()) {
        cout << endl << "Test Other Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of a hash for verification
    simulateSecondMessage();
    // Perform verification on received hash
    if (!replyToSecondMessage()) {
        cout << endl << "Test Other Way FAILED!!!!" << endl << endl;
        return false;
    }
    // Simulate reception of encrypted facial recognition params + hash
    simulateThirdMessage();
    // Corrupt the third message reply!!!!
    corruptThirdMessageReply();
    // Decrypt and verify the third message
    if (!replyToThirdMessage()) {
        cout << endl << "Other Way Corruption Test Passed!" << endl << endl;
        return true;
    } else {
        cout << endl << "Other Way Corruption Test FAILED!!!!!!" << endl << endl;
        return false;
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
                cout << "Tests are numbered 1-4, please re-enter your input and try again." 
                        << endl;
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
    cout << endl << endl << "ALL TESTS PASSED!!!!!" << endl;
    return 0;
}