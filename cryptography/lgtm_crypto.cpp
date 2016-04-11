// g++ -std=c++11 -g3 -ggdb -O0 -DDEBUG lgtm-crypto.cpp -o lgtm-crypto -lcryptopp -lpthread

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

void diffieHellmanSharedSecretAgreement(SecByteBlock &sharedSecret, SecByteBlock &otherPublicKey, 
        SecByteBlock &privateKey) {
    OID CURVE = secp256r1();

    ECDH<ECP>::Domain diffieHellman(CURVE);
    if (!diffieHellman.Agree(sharedSecret, privateKey, otherPublicKey)) {
        // Do something bad, maybe return false
    }
}

void generateSymmetricKeyFromSharedSecret(SecByteBlock &key, SecByteBlock &sharedSecret) {
    key.CleanNew(SHA256::DIGESTSIZE);
    SHA256().CalculateDigest(key, sharedSecret.BytePtr(), sharedSecret.SizeInBytes());
}

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
    // Get file length and read it all into one byte buffer
    authInputStream.seekg(0, authInputStream.end);
    int authDataLength = authInputStream.tellg();
    authInputStream.seekg(0, authInputStream.beg);
    byte *authData = new byte[authDataLength];
    authInputStream.read((char*) authData, authDataLength);
    authInputStream.close();

    encryptionFilter.ChannelPut(AAD_CHANNEL, authData, authDataLength);
    encryptionFilter.ChannelMessageEnd(AAD_CHANNEL);
    
    // Authenticated data *must* be pushed before
    //  Confidential/Authenticated data. Otherwise
    //  we must catch the BadState exception
    // Read input data from passed file name
    ifstream inputStream(inputFileName, ios::in | ios::binary);
    // Get file length and read it all into one byte buffer
    inputStream.seekg(0, inputStream.end);
    int inputDataLength = inputStream.tellg();
    inputStream.seekg(0, inputStream.beg);
    byte *inputData = new byte[inputDataLength];
    inputStream.read((char*) inputData, inputDataLength);
    inputStream.close();

    encryptionFilter.ChannelPut(DEFAULT_CHANNEL, inputData, inputDataLength);
    encryptionFilter.ChannelMessageEnd(DEFAULT_CHANNEL);


    delete[] authData;
    delete[] inputData;

    // Read from file and write back to file
    //FileSource fileSource(inputFileName, true, 
    //        new StreamTransformationFilter(encrypt, new FileSink(outputFileName)));
}

void decryptFile(const string &inputFileName, const string &outputFileName, 
        const string &authInputFileName, SecByteBlock &key, byte *ivBytes) {
    GCM<AES>::Decryption decrypt;
    decrypt.SetKeyWithIV(key, key.size(), ivBytes, AES::BLOCKSIZE);

    AuthenticatedDecryptionFilter decryptionFilter(decrypt, 
            new FileSink(outputFileName.c_str()), 
            AuthenticatedDecryptionFilter::MAC_AT_BEGIN | 
            AuthenticatedDecryptionFilter::THROW_EXCEPTION, MAC_SIZE);

    // Read cipher text from inputFileName
    ifstream inputStream(inputFileName, ios::in | ios::binary);
    // Get file length and read it all into one byte buffer
    inputStream.seekg(0, inputStream.end);
    int inputDataLength = inputStream.tellg();
    inputStream.seekg(0, inputStream.beg);
    byte *inputData = new byte[inputDataLength];
    inputStream.read((char*) inputData, inputDataLength);
    inputStream.close();

    // Read authentication bytes from authInputFileName
    ifstream authInputStream(authInputFileName, ios::in | ios::binary);
    // Get file length and read it all into one byte buffer
    authInputStream.seekg(0, authInputStream.end);
    int authInputDataLength = authInputStream.tellg();
    authInputStream.seekg(0, authInputStream.beg);
    byte *authInputData = new byte[authInputDataLength];
    inputStream.read((char*) authInputData, authInputDataLength);
    inputStream.close();

    byte macBytes[MAC_SIZE];
    // The order of the following calls are important
    decryptionFilter.ChannelPut(DEFAULT_CHANNEL, macBytes, MAC_SIZE);
    decryptionFilter.ChannelPut(AAD_CHANNEL, authInputData, authInputDataLength); 
    decryptionFilter.ChannelPut(DEFAULT_CHANNEL, inputData, inputDataLength);   

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

    ofstream outputStream(outputFileName, ios::in | ios::binary);
    outputStream.write((char*) retrievedData, numBytesToRetrieve);
    outputStream.close();

    delete[] inputData;
    delete[] authInputData;
    delete[] retrievedData;

    // Read from file and write back to file
    //FileSource fileSource(inputFileName, true, 
    //        new StreamTransformationFilter(decrypt, new FileSink(outputFileName)));
}

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

void verifyMacForFile(const string &fileName, const string &macInputFileName) {
    
}