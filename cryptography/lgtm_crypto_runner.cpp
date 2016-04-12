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

#include "lgtm_crypto_runner.hpp"

//~Function Headers---------------------------------------------------------------------------------
static void readFromFile(const string &fileName, SecByteBlock &input);
static void writeToFile(const string &fileName, SecByteBlock &output);

//~Constants----------------------------------------------------------------------------------------
static const string LGTM_CRYPTO_PREFIX = ".lgtm-crypto-params-";
// Crypto parameters file names
static const string PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "public-key";
static const string PRIVATE_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "private-key";
static const string OTHER_PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-public-key";
static const string SHARED_SECRET_FILE_NAME = LGTM_CRYPTO_PREFIX + "shared-secret";
static const string COMPUTED_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "computed-key";
static const string VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "verification-hash";
static const string OTHER_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-verification-hash";
static const string FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "facial-recognition-verification-hash";
static const string OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX + "facial-recognition-other-verification-hash";
static const string CURRENT_IV = LGTM_CRYPTO_PREFIX + "initialization-vector";

// Message file names
static const string FIRST_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message";
static const string FIRST_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message-reply";

static const string SECOND_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "second-message";
static const string SECOND_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "second-message-reply";

static const string THIRD_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message";
static const string THIRD_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message-reply";

// Other LGTM file names
static const string FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params";
static const string ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params--encrypted";
static const string RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-received-facial-recognition-params";
static const string DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-received-facial-recognition-params--decrypted";

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
}

/**
 * Create Diffie-Hellman parameters and save them to files.
 * Compute the shared secret and computed key from the received other public key.
 * Save shared secret and computed key to file.
 */
void replyToFirstMessage() {
    cout << "Reply To First Message" << endl;
    // Read in received Diffie-Hellman public key from file
    SecByteBlock otherPublicKey;
    readFromFile(OTHER_PUBLIC_KEY_FILE_NAME, otherPublicKey);

    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);

    // Compute shared secret
    SecByteBlock sharedSecret;
    if (!diffieHellmanSharedSecretAgreement(sharedSecret, otherPublicKey, privateKey)) {
        throw runtime_error("ERROR!! INTRUDER ALERT!!");
    }

    // Compute key from shared secret
    SecByteBlock key;
    generateSymmetricKeyFromSharedSecret(key, sharedSecret);

    // Write to file
    writeToFile(SHARED_SECRET_FILE_NAME, sharedSecret);
    writeToFile(COMPUTED_KEY_FILE_NAME, key);
}

/**
 * Compute shared secret and computed key using the received other's public key.
 * Compute hash over all prior messages + a random number which is also saved. TODO: (the last bit)
 * Save hash and random number to separate files.
 * Encrypt hash and save to another file.
 */
void secondMessage() {
    // TODO: Add some encryption here....
    cout << "Second Message" << endl;
    // Read from file
    SecByteBlock privateKey;
    SecByteBlock otherPublicKey;
    readFromFile(PRIVATE_KEY_FILE_NAME, privateKey);
    readFromFile(OTHER_PUBLIC_KEY_FILE_NAME, otherPublicKey);

    // Compute shared secret
    SecByteBlock sharedSecret;
    diffieHellmanSharedSecretAgreement(sharedSecret, otherPublicKey, privateKey);

    // Compute key from shared secret
    SecByteBlock key;
    generateSymmetricKeyFromSharedSecret(key, sharedSecret);

    // Generate random number to be included in body of message
    // SecByteBlock randomNumber;
    // TODO: 

    // Compute hash over all prior messages + this (Other public key + my public key)
    vector<string> hashFiles;
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    createHashFromFiles(hashFiles, VERIFICATION_HASH_FILE_NAME);

    // Write to file
    writeToFile(SHARED_SECRET_FILE_NAME, sharedSecret);
    writeToFile(COMPUTED_KEY_FILE_NAME, key);
    // TODO:
    // writeToFile(SECOND_MESSAGE_RANDOM_NUMBER, randomNumber);
}

/**
 * Decrypt hash received from other user.
 * Validate hash.
 * Compute hash over all prior messages + a random number which is also saved. TODO: (the last bit)
 * Save hash and random number to separate files.
 * Encrypt hash and save to file to prepare for sending.
 */
void replyToSecondMessage() {
    // TODO: Add some encryption here....
    cout << "Reply To Second Message" << endl;
    // Read from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);
    // TODO:
    // SecByteBlock otherRandomNumber
    // readFromFile(OTHER_SECOND_MESSAGE_RANDOM_NUMBER, otherRandomNumber)

    // Verify received Mac
    vector<string> verifyhashFiles;
    verifyhashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyhashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    if (!verifyHashFromFiles(verifyhashFiles, OTHER_VERIFICATION_HASH_FILE_NAME)) {
        throw runtime_error("FAILED TO VERIFY HASH! INTRUDER ALERT!");
    }

    // Generate random number to be included in body of message
    // SecByteBlock randomNumber;
    // TODO: 

    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    createHashFromFiles(hashFiles, VERIFICATION_HASH_FILE_NAME);

    // TODO:
    // writeToFile(SECOND_MESSAGE_RANDOM_NUMBER, randomNumber);
}

/**
 * Decrypt hash received from other user.
 * Verify hash.
 * Compute hash over all prior messages + facial recognition params.
 * Encrypt facial recognition parameters.
 */
void thirdMessage() {
    // TODO: Add encrypt/decrypt....
    cout << "Third Message" << endl;
    // Read session key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Verify received HASH
    // TODO: add some randomness to the messages + hash
    vector<string> verifyhashFiles;
    verifyhashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyhashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    verifyhashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    if (!verifyHashFromFiles(verifyhashFiles, OTHER_VERIFICATION_HASH_FILE_NAME)) {
        throw runtime_error("FAILED TO VERIFY HASH! INTRUDER ALERT!");
    }

    // Compute HASH of all prior messages + this
    // TODO: add some randomness to the messages + hash
    vector<string> hashFiles;
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    createHashFromFiles(hashFiles, FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);

    // Encrypt (facial recognition params + HASH)
    encryptFile(FACIAL_RECOGNITION_FILE_NAME, "", 
            ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);
}

/**
 * Decrypt hash received from other user.
 * Verify hash.
 * Compute hash over all prior messages + facial recognition params.
 * Encrypt facial recognition parameters.
 */
void replyToThirdMessage() {
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
    decryptFile(RECEIVED_FACIAL_RECOGNITION_FILE_NAME, "", 
            DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);

    // Verify received HASH
    vector<string> verifyHashFiles;
    verifyHashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFromFiles(verifyHashFiles, OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);

    // Compute HASH of all prior messages + this
    vector<string> hashFiles;
    hashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    hashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    hashFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    hashFiles.push_back(OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    createHashFromFiles(hashFiles, FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);

    // Encrypt (facial recognition params + HASH)
    encryptFile(FACIAL_RECOGNITION_FILE_NAME, "", ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);
}

/**
 * 
 */
void decryptThirdMessageReply() {
    cout << "Decrypt Reply To Third Message" << endl;
    // Read session key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Decrypt (facial recognition params + HASH)
    decryptFile(RECEIVED_FACIAL_RECOGNITION_FILE_NAME, "", 
            DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);

    //decryptFile(OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME)

    // Verify received HASH
    vector<string> verifyHashFiles;
    verifyHashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFiles.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    verifyHashFromFiles(verifyHashFiles, OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
}

/**
 * Reads into a SecByteBlock from a file specified by fileName.
 */
static void readFromFile(const string &fileName, SecByteBlock &input) {
    ifstream inputStream(fileName, ios::in | ios::binary);
    if (inputStream.is_open()) {
        // Get file length
        inputStream.seekg(0, inputStream.end);
        int fileLength = inputStream.tellg();
        inputStream.seekg(0, inputStream.beg);
        // Check if the file has anything in it.
        if (fileLength > 0) {
            input.CleanNew(fileLength);
            inputStream.read((char*) input.BytePtr(), input.SizeInBytes());
        } else {
            cerr << "Error, file length is: " << fileLength << " for file: " << fileName << endl
                    << "Disregarding and continuing...." << endl;
        }
        inputStream.close();
    } else {
        cerr << "Error in readFromFile" << endl << "file: " << fileName 
                << " could not be opened in readFromFile in lgtm_crypto_runner.cpp "<< endl 
                << "Disregarding and continuing...." << endl;
    }
    cout << "Read from file, read " << input.SizeInBytes() 
            << " bytes from file: " << fileName << endl;
}

/**
 * Writes a SecByteBlock to a file specified by fileName.
 */
static void writeToFile(const string &fileName, SecByteBlock &output) {
    cout << "Write to file, writing " << output.SizeInBytes() 
            << " bytes to file: " << fileName << endl;
    ofstream outputStream(fileName, ios::out | ios::binary);
    outputStream.write((char*) output.BytePtr(), output.SizeInBytes());
    outputStream.close();
}

//~Main function------------------------------------------------------------------------------------
/*
int main(int argc, char *argv[]) {

    if (argc != 2) {
        return 1; }

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
    // First message
    // Reply to first message
    // Second message
    // Reply to second message
    // Encrypt
    return 0;
}
*/