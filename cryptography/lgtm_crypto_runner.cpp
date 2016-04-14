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

//~Constants----------------------------------------------------------------------------------------
static const string LGTM_CRYPTO_PREFIX = ".lgtm-crypto-params-";
// Crypto parameters file names
static const string PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "public-key";
static const string PRIVATE_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "private-key";
static const string OTHER_PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-public-key";
static const string SHARED_SECRET_FILE_NAME = LGTM_CRYPTO_PREFIX + "shared-secret";
static const string COMPUTED_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "computed-key";
static const string CURRENT_IV = LGTM_CRYPTO_PREFIX + "initialization-vector";

// Hash verification files
static const string VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "verification-hash";
static const string ENCRYPTED_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "encrypted-verification-hash";

static const string OTHER_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "other-verification-hash";
static const string ENCRYPTED_OTHER_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "encrypted-other-verification-hash";

static const string FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "facial-recognition-verification-hash";
static const string OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME = LGTM_CRYPTO_PREFIX 
        + "facial-recognition-other-verification-hash";

// Message file names
static const string FIRST_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message";
static const string FIRST_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "first-message-reply";

static const string SECOND_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "second-message";
static const string SECOND_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "second-message-reply";

static const string THIRD_MESSAGE_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message";
static const string THIRD_MESSAGE_REPLY_FILE_NAME = LGTM_CRYPTO_PREFIX + "third-message-reply";

// Other LGTM file names
static const string VERIFIED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params-with-hash";

static const string FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params";
static const string ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params--encrypted";

//static const string RECEIVED_VERIFIED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-received-facial-recognition-params-with-hash";
static const string RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-received-facial-recognition-params";
static const string DECRYPTED_RECEIVED_VERIFIED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-received-facial-recognition-params-with-hash--decrypted";
static const string DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME 
        = ".lgtm-received-facial-recognition-params--decrypted";

// Crypto parameters
static const int HASH_NUM_BYTES = 64;

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
bool replyToFirstMessage() {
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
        cerr << "Security Error in replyToFirstMessage. Diffie-Hellman shared secret could not be agreed to." << endl;
        return false;
    }

    // Compute key from shared secret
    SecByteBlock key;
    generateSymmetricKeyFromSharedSecret(key, sharedSecret);

    // Write to file
    writeToFile(SHARED_SECRET_FILE_NAME, sharedSecret);
    writeToFile(COMPUTED_KEY_FILE_NAME, key);
    return true;
}

/**
 * Compute shared secret and computed key using the received other's public key.
 * Compute hash over all prior messages + a random number which is also saved. TODO: (the last bit)
 * Save hash and random number to separate files.
 * Encrypt hash and save to another file.
 */
bool secondMessage() {
    cout << "Second Message" << endl;
    // Read from file
    SecByteBlock privateKey;
    SecByteBlock otherPublicKey;
    readFromFile(PRIVATE_KEY_FILE_NAME, privateKey);
    readFromFile(OTHER_PUBLIC_KEY_FILE_NAME, otherPublicKey);

    // Compute shared secret
    SecByteBlock sharedSecret;
    if (!diffieHellmanSharedSecretAgreement(sharedSecret, otherPublicKey, privateKey)) {
        cerr << "Security Error in secondMessage. Diffie-Hellman shared secret could not be agreed to." << endl;
        return false;
    }

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

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    encryptFile(VERIFICATION_HASH_FILE_NAME, 
            ENCRYPTED_VERIFICATION_HASH_FILE_NAME, 
            key, curIv);

    // Write to file
    writeToFile(SHARED_SECRET_FILE_NAME, sharedSecret);
    writeToFile(COMPUTED_KEY_FILE_NAME, key);
    // TODO:
    // writeToFile(SECOND_MESSAGE_RANDOM_NUMBER, randomNumber);
    return true;
}

/**
 * Decrypt hash received from other user.
 * Validate hash.
 * Compute hash over all prior messages + a random number which is also saved. TODO: (the last bit)
 * Save hash and random number to separate files.
 * Encrypt hash and save to file to prepare for sending.
 */
bool replyToSecondMessage() {
    cout << "Reply To Second Message" << endl;
    // Read from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);
    // TODO:
    // SecByteBlock otherRandomNumber
    // readFromFile(OTHER_SECOND_MESSAGE_RANDOM_NUMBER, otherRandomNumber)

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Decrypt hash file
    if (!decryptFile(ENCRYPTED_OTHER_VERIFICATION_HASH_FILE_NAME, 
            OTHER_VERIFICATION_HASH_FILE_NAME,
            key, curIv)) {
        cerr << "Security Error in replyToSecondMessage. MAC could not be verified." << endl;
        return false;
    }

    // Verify received hash
    vector<string> verifyHashFiles;
    verifyHashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    if (!verifyHashFromFiles(verifyHashFiles, OTHER_VERIFICATION_HASH_FILE_NAME)) {
        cerr << "Security Error in replyToSecondMessage. Hash could not be verified." << endl;
        return false;
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

    // Encrypt hash for transmission
    encryptFile(VERIFICATION_HASH_FILE_NAME, 
            ENCRYPTED_VERIFICATION_HASH_FILE_NAME,
            key, curIv);

    // TODO:
    // writeToFile(SECOND_MESSAGE_RANDOM_NUMBER, randomNumber);
    return true;
}

/**
 * Decrypt hash received from other user.
 * Verify hash.
 * Compute hash over all prior messages + facial recognition params.
 * Encrypt facial recognition parameters.
 */
bool thirdMessage() {
    cout << "Third Message" << endl;
    // Read session key from file
    SecByteBlock key;
    readFromFile(COMPUTED_KEY_FILE_NAME, key);

    // Read in the current initialization vector from file
    byte curIv[AES::BLOCKSIZE];
    // TODO: actually read it in
    // Set to 0 for now
    memset(curIv, 0, AES::BLOCKSIZE);

    // Decrypt received second message
    // which should be a hash of all prior messages + a random number TODO: (the last bit)
    if (!decryptFile(ENCRYPTED_OTHER_VERIFICATION_HASH_FILE_NAME, 
            OTHER_VERIFICATION_HASH_FILE_NAME,
            key, curIv)) {
        cerr << "Security Error in thirdMessage. MAC could not be verified." << endl;
        return false;
    }

    // Verify received HASH
    // TODO: add some randomness to the messages + hash
    vector<string> verifyHashFiles;
    verifyHashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    if (!verifyHashFromFiles(verifyHashFiles, OTHER_VERIFICATION_HASH_FILE_NAME)) {
        cerr << "Security Error in thirdMessage. Hash could not be verified." << endl;
        return false;
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

    // Combine facial recognition params + Hash
    // TODO: 
    vector<string> fileNames;
    fileNames.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    fileNames.push_back(FACIAL_RECOGNITION_FILE_NAME);
    combineFiles(fileNames, VERIFIED_FACIAL_RECOGNITION_FILE_NAME);

    // Encrypt (facial recognition params + HASH)
    encryptFile(VERIFIED_FACIAL_RECOGNITION_FILE_NAME,
            ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv);
    return true;
}

/**
 * Decrypt hash received from other user.
 * Verify hash.
 * Compute hash over all prior messages + facial recognition params.
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
    if (!decryptFile(RECEIVED_FACIAL_RECOGNITION_FILE_NAME,
            DECRYPTED_RECEIVED_VERIFIED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv)) {
        cerr << "Security Error in replyToThirdMessage. MAC could not be verified." << endl;
        return false;
    }

    // Split off hash from DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME into a separate file
    vector<string> outputFiles;
    outputFiles.push_back(OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
        //OTHER_VERIFICATION_HASH_FILE_NAME);
    outputFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    vector<int> bytesPerFile;
    bytesPerFile.push_back(HASH_NUM_BYTES);
    splitFile(DECRYPTED_RECEIVED_VERIFIED_FACIAL_RECOGNITION_FILE_NAME, outputFiles, bytesPerFile);

    // Verify received HASH
    vector<string> verifyHashFiles;
    verifyHashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFiles.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    // TODO: cleanup
    /*
    cout << "Printing all files in hash in replyToThirdMessage: " << endl;
    for (int i = 0; i < verifyHashFiles.size(); i++) {
        cout << "File Name: " << verifyHashFiles[i] << endl;
        printFile(verifyHashFiles[i]);
    }
    cout << "*****DONE PRINTING ALL FILES IN HASH IN replyToThirdMessage" << endl;
    */
    if (!verifyHashFromFiles(verifyHashFiles, 
            OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME)) {
        cerr << "Security Error in replyToThirdMessage. Hash could not be verified." << endl;
        return false;
    }

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

    // Combine facial recognition params + Hash
    vector<string> fileNames;
    fileNames.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    fileNames.push_back(FACIAL_RECOGNITION_FILE_NAME);
    combineFiles(fileNames, VERIFIED_FACIAL_RECOGNITION_FILE_NAME);

    // Encrypt (facial recognition params + HASH)
    encryptFile(VERIFIED_FACIAL_RECOGNITION_FILE_NAME, 
            ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME,
            key, curIv);
    return true;
}

/**
 * 
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

    // Decrypt (facial recognition params + HASH)
    if (!decryptFile(RECEIVED_FACIAL_RECOGNITION_FILE_NAME, 
            DECRYPTED_RECEIVED_VERIFIED_FACIAL_RECOGNITION_FILE_NAME, 
            key, curIv)) {
        cerr << "Security Error in decryptThirdMessageReply. MAC could not be verified." << endl;
        return false;
    }


    // Split off hash from DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME into a separate file
    vector<string> outputFiles;
    outputFiles.push_back(OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    outputFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    vector<int> bytesPerFile;
    bytesPerFile.push_back(HASH_NUM_BYTES);
    splitFile(DECRYPTED_RECEIVED_VERIFIED_FACIAL_RECOGNITION_FILE_NAME, outputFiles, bytesPerFile);

    // Verify received HASH
    vector<string> verifyHashFiles;
    verifyHashFiles.push_back(PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(OTHER_PUBLIC_KEY_FILE_NAME);
    verifyHashFiles.push_back(VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(OTHER_VERIFICATION_HASH_FILE_NAME);
    verifyHashFiles.push_back(FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFiles.push_back(DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME);
    verifyHashFiles.push_back(FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME);
    if (!verifyHashFromFiles(verifyHashFiles, 
            OTHER_FACIAL_RECOGNITION_VERIFICATION_HASH_FILE_NAME)) {
        cerr << "Security Error in decryptThirdMessageReply. Hash could not be verified." << endl;
        return false;
    }

    return true;
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