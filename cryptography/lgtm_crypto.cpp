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

// g++ -std=c++11 -g3 -ggdb -O0 -I . lgtm_crypto.cpp ../../cryptopp/libcryptopp.a -o lgtm_crypto -lcryptopp -static -lpthread

#include "lgtm_crypto.hpp"

//~Constants----------------------------------------------------------------------------------------
static const int MAC_SIZE = 12;

//~Functions----------------------------------------------------------------------------------------
/**
 * Generate Diffie-Hellman Public-Private keys.
 */
void generateDiffieHellmanParameters(SecByteBlock &publicKey, SecByteBlock &privateKey) {
    OID CURVE = secp256r1();

    ECDH<ECP>::Domain diffieHellman(CURVE);
    AutoSeededRandomPool rng;
    publicKey.CleanNew(diffieHellman.PublicKeyLength());
    privateKey.CleanNew(diffieHellman.PrivateKeyLength());
    diffieHellman.GenerateKeyPair(rng, privateKey, publicKey);
}

/**
 * Derive the shared secret from a private key and another party's public key.
 */
void diffieHellmanSharedSecretAgreement(SecByteBlock &sharedSecret, SecByteBlock &otherPublicKey, 
        SecByteBlock &privateKey) {
    if (otherPublicKey.SizeInBytes() == 0) {
        throw runtime_error("Other user's public key is empty!!!");
    }
    if (privateKey.SizeInBytes() == 0) {
        throw runtime_error("Private key is empty!!!");
    }
    OID CURVE = secp256r1();

    ECDH<ECP>::Domain diffieHellman(CURVE);
    sharedSecret.CleanNew(diffieHellman.AgreedValueLength());
    if (!diffieHellman.Agree(sharedSecret, privateKey, otherPublicKey)) {
        // TODO:
        // Do something bad, maybe return false
    }
}

/**
 * Take some shared secret and compute a 256 bit hash over it to generate a key.
 */
void generateSymmetricKeyFromSharedSecret(SecByteBlock &key, SecByteBlock &sharedSecret) {
    if (sharedSecret.SizeInBytes() == 0) {
        throw runtime_error("Shared secret is empty!!!");
    }
    key.CleanNew(SHA256::DIGESTSIZE);
    SHA256().CalculateDigest(key, sharedSecret.BytePtr(), sharedSecret.SizeInBytes());
}

/**
 * Encrypt with ADDITIONALLY authenticated (but unencrypted) data.
 *
 * Take an inputFileName and encrypt it, placing the results in the outputFileName.
 * The encryption process is authenticated and takes secondary authentication information
 * from the authInputFileName.
 *
 * The encryption algorithm uses key and ivBytes to perform GCM<AES> encryption.
 * ivBytes is 256 bits (the size of AES::BLOCKSIZE).
 */
void encryptFile(const string &inputFileName, const string &authInputFileName, 
        const string &outputFileName, /*const string &authOutputFileName, (Add back later?)*/
        SecByteBlock &key, byte *ivBytes) {
    GCM<AES>::Encryption encrypt;
    encrypt.SetKeyWithIV(key, key.size(), ivBytes, AES::BLOCKSIZE);

    AuthenticatedEncryptionFilter encryptionFilter(encrypt, 
            new FileSink(outputFileName.c_str()), false, MAC_SIZE);

    // AuthenticatedEncryptionFilter::ChannelPut
    //  defines two channels: DEFAULT_CHANNEL and AAD_CHANNEL
    //   DEFAULT_CHANNEL is encrypted and authenticated
    //   AAD_CHANNEL is authenticated
    // Read auth data from passed file name
    ifstream authInputStream(authInputFileName, ios::in | ios::binary);
    // Check if we succeeded in opening the file....
    if (authInputStream.is_open()) {
        // Get file length and read it all into one byte buffer
        authInputStream.seekg(0, authInputStream.end);
        int authDataLength = authInputStream.tellg();
        authInputStream.seekg(0, authInputStream.beg);
        byte *authData = new byte[authDataLength];
        authInputStream.read((char*) authData, authDataLength);
        authInputStream.close();

        encryptionFilter.ChannelPut(AAD_CHANNEL, authData, authDataLength);
        encryptionFilter.ChannelMessageEnd(AAD_CHANNEL);

        delete[] authData;
    } else {
        cerr << "Error opening authentication file: " << authInputFileName 
                << " in decryptFile in lgtm_crypto.cpp." << endl
                << "Continuing on....." << endl;
    }
    
    // Authenticated data *must* be pushed before
    //  Confidential/Authenticated data. Otherwise
    //  we must catch the BadState exception
    // Read input data from passed file name
    ifstream inputStream(inputFileName, ios::in | ios::binary);
    if (inputStream.is_open()) {
        // Get file length and read it all into one byte buffer
        inputStream.seekg(0, inputStream.end);
        int inputDataLength = inputStream.tellg();
        inputStream.seekg(0, inputStream.beg);
        byte *inputData = new byte[inputDataLength];
        inputStream.read((char*) inputData, inputDataLength);
        inputStream.close();

        encryptionFilter.ChannelPut(DEFAULT_CHANNEL, inputData, inputDataLength);
        encryptionFilter.ChannelMessageEnd(DEFAULT_CHANNEL);

        delete[] inputData;
    } else {
        cerr << "Error opening file: " << inputFileName 
                << " in encryptFile in lgtm_crypto.cpp" << endl;
        exit(1);
    }
}

/**
 * Decrypt with ADDITIONALLY authenticated (but unencrypted) data.
 *
 * Take an inputFileName and decrypts it, placing the results in the outputFileName.
 * The encryption process is authenticated and takes secondary authentication information
 * from the authInputFileName.
 *
 * The encryption algorithm uses key and ivBytes to perform GCM<AES> encryption.
 * ivBytes is 256 bits (the size of AES::BLOCKSIZE).
 */
void decryptFile(const string &inputFileName, const string &authInputFileName, 
        const string &outputFileName, 
        SecByteBlock &key, byte *ivBytes) {
    GCM<AES>::Decryption decrypt;
    decrypt.SetKeyWithIV(key, key.size(), ivBytes, AES::BLOCKSIZE);
    AuthenticatedDecryptionFilter decryptionFilter(decrypt, 
            NULL, /*new FileSink(outputFileName.c_str()), */
            AuthenticatedDecryptionFilter::MAC_AT_BEGIN | 
            AuthenticatedDecryptionFilter::THROW_EXCEPTION, MAC_SIZE);

    // Read cipher text from inputFileName
    ifstream inputStream(inputFileName, ios::in | ios::binary);
    if (inputStream.is_open()) {
        // Get length of the encrypted data and read the data into two different buffers
        // one for encrypted data, one for the mac data
        inputStream.seekg(0, inputStream.end);
        int fileLength = inputStream.tellg();
        int encryptedDataLength = (fileLength - MAC_SIZE);
        inputStream.seekg(0, inputStream.beg);
        if (encryptedDataLength < 1) {
            cerr << "Cannot allocate a negative number of bytes: " << encryptedDataLength
                    << " in decryptFile in lgtm_crypto.cpp" << endl
                    << "Actual file length was: " << fileLength << endl;
            exit(1);
        }
        byte *encryptedData = new byte[encryptedDataLength];
        if (!encryptedData) {
            cerr << "Error allocating bytes for inputData in decryptFile in lgtm_crypto.cpp" 
                    << endl;
            exit(1);
        }
        // Allocate memory for the MAC
        byte macData[MAC_SIZE];
        // Read the encrypted data
        inputStream.read((char*) encryptedData, encryptedDataLength);
        // Read the MAC bytes
        inputStream.read((char*) macData, MAC_SIZE);
        inputStream.close();

        // The order of the "ChannelPut" calls is important!!! 
        // (MAC -> AUTHENTICATED DATA -> ENCRYPTED DATA)
        decryptionFilter.ChannelPut(DEFAULT_CHANNEL, macData, MAC_SIZE);

        // Read authentication bytes from authInputFileName
        ifstream authInputStream(authInputFileName, ios::in | ios::binary);
        if (authInputStream.is_open()) {
            // Get file length and read it all into one byte buffer
            authInputStream.seekg(0, authInputStream.end);
            int authInputDataLength = authInputStream.tellg();
            authInputStream.seekg(0, authInputStream.beg);
            byte *authInputData = new byte[authInputDataLength];
            if (!authInputData) {
                cerr << "Error allocating bytes for authInputData in decryptFile in lgtm_crypto.cpp" 
                        << endl;
                exit(1);
            }
            inputStream.read((char*) authInputData, authInputDataLength);
            inputStream.close();

            // Read out the additional authenticated data
            decryptionFilter.ChannelPut(AAD_CHANNEL, authInputData, authInputDataLength); 
            decryptionFilter.ChannelMessageEnd(AAD_CHANNEL);
            delete[] authInputData;

        } else {
            cerr << "Error opening authentication file: " << authInputFileName 
                    << " in decryptFile in lgtm_crypto.cpp" << endl
                    << "Continuing on....." << endl;
        }

        // Read out the encryptedData
        decryptionFilter.ChannelPut(DEFAULT_CHANNEL, encryptedData, encryptedDataLength);   
        delete[] encryptedData;

    } else {
        cerr << "Error opening file: " << inputFileName 
                << " in decryptFile in lgtm_crypto.cpp" << endl;
        exit(1);
    }

    // If the object throws, it will most likely occur
    //   during ChannelMessageEnd()
    decryptionFilter.ChannelMessageEnd(AAD_CHANNEL);
    decryptionFilter.ChannelMessageEnd(DEFAULT_CHANNEL);

    // Verify authentication
    if (!decryptionFilter.GetLastResult()) {
        // do something with verification
    }

    decryptionFilter.SetRetrievalChannel(DEFAULT_CHANNEL);
    int numBytesToRetrieve = decryptionFilter.MaxRetrievable();
    byte *retrievedData = new byte[numBytesToRetrieve];
    if (numBytesToRetrieve > 0) {
        decryptionFilter.Get(retrievedData, numBytesToRetrieve);
    }

    cout << "decryptFile, writing " << numBytesToRetrieve << " bytes to file: " 
            << outputFileName << endl;
    ofstream outputStream(outputFileName, ios::out | ios::binary);
    outputStream.write((char*) retrievedData, numBytesToRetrieve);
    outputStream.close();

    delete[] retrievedData;
}

/**
 * Generates a HMAC from inputFileName and places the result in outputFileName.
 */
void createMacForFile(const string &inputFileName, const string &outputFileName) {
    try {
        HMAC<SHA512> hmac;
        // Read from file and write back to file
        FileSource fileSource(inputFileName.c_str(), true, 
                new HashFilter(hmac, 
                    new FileSink(outputFileName.c_str())));
    } catch (const CryptoPP::Exception &e) {
        cout << "Issue in creatMac." << endl;
        cerr << e.what() << endl;
        exit(1);
    }
}

/**
 * Verifies a HMAC from macInputFileName by comparing it to a HMAC over the data in fileName.
 */
void verifyMacForFile(const string &fileName, const string &macInputFileName) {
    
}