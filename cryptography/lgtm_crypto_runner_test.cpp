// g++ -std=c++11 -g3 -ggdb -O0 -DDEBUG lgtm_crypto.cpp lgtm_crypto_runner.cpp lgtm_crypto_runner_test.cpp -o lgtm_crypto_runner_test -lcryptopp -lpthread

#include "lgtm_crypto.hpp"
#include "lgtm_crypto_runner.hpp"

#include "../../cryptopp/secblock.h"

#include <fstream>
#include <string>

using CryptoPP::SecByteBlock;

using std::ios;
using std::ofstream;
using std::string;

//~Global variables---------------------------------------------------------------------------------
static const string LGTM_CRYPTO_PREFIX = ".lgtm-crypto-params-";

// "Other" Crypto params
static const string OTHER_PUBLIC_KEY_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-public-key";
static const string OTHER_VERIFICATION_MAC_FILE_NAME = LGTM_CRYPTO_PREFIX + "other-verification-mac";

// Facial recognition files
static const string FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params";
static const string ENCRYPTED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params--encrypted";
static const string RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-received-facial-recognition-params";
static const string DECRYPTED_RECEIVED_FACIAL_RECOGNITION_FILE_NAME = ".lgtm-facial-recognition-params--decrypted";

/**
 * Writes a SecByteBlock to a file specified by fileName.
 */
static void writeToFile(const string &fileName, SecByteBlock &output) {
    ofstream outputStream(fileName, ios::out | ios::binary);
    outputStream.write((char*) output.BytePtr(), output.SizeInBytes());
    outputStream.close();
}

void simulateFirstMessageReply() {
    // Prepare Diffie-Hellman parameters
    SecByteBlock publicKey;
    SecByteBlock privateKey;
    generateDiffieHellmanParameters(publicKey, privateKey);
    // Write public key and private key to files
    writeToFile(OTHER_PUBLIC_KEY_FILE_NAME, publicKey);
}

void testOneWay() {
    firstMessage();
    simulateFirstMessageReply();
    secondMessage();
    thirdMessage();
    decryptThirdMessageReply();
}

void testOtherWay() {
    simulateFirstMessageReply();
    replyToFirstMessage();
    replyToSecondMessage();
    replyToThirdMessage();
}

int main() {
    testOneWay();
    // testOtherWay();
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