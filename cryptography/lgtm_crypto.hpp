#ifndef LGTM_CRYPTO_H_
#define LGTM_CRYPTO_H_

#include "../../cryptopp/aes.h"
#include "../../cryptopp/asn.h"
//#include "../../cryptopp/cryptlib.h"
#include "../../cryptopp/eccrypto.h"
#include "../../cryptopp/files.h"
#include "../../cryptopp/filters.h"
#include "../../cryptopp/gcm.h"
#include "../../cryptopp/hex.h"
#include "../../cryptopp/hmac.h"
#include "../../cryptopp/oids.h"
#include "../../cryptopp/osrng.h"
#include "../../cryptopp/secblock.h"
#include "../../cryptopp/sha.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace CryptoPP::ASN1;

using CryptoPP::AAD_CHANNEL;
using CryptoPP::AES;
using CryptoPP::AuthenticatedDecryptionFilter;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::CBC_Mode;
using CryptoPP::ECDH;
using CryptoPP::ECP;
using CryptoPP::DEFAULT_CHANNEL;
using CryptoPP::FileSink;
using CryptoPP::FileSource;
using CryptoPP::GCM;
using CryptoPP::GCM_TablesOption;
using CryptoPP::HashFilter;
using CryptoPP::HMAC;
using CryptoPP::OID;
using CryptoPP::SecByteBlock;
using CryptoPP::SHA256;
using CryptoPP::SHA512;

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;

//~Function Headers---------------------------------------------------------------------------------
// Diffie-Hellman
void generateDiffieHellmanParameters(SecByteBlock &publicKey, SecByteBlock &privateKey);
void diffieHellmanSharedSecretAgreement(SecByteBlock &sharedSecret, SecByteBlock &otherPublicKey, 
        SecByteBlock &privateKey);
void generateSymmetricKeyFromSharedSecret(SecByteBlock &key, SecByteBlock &sharedSecret);
// Encryption
void encryptFile(const string &inputFileName, const string &authInputFileName, 
        const string &outputFileName, /*const string &authOutputFileName, (Add back later?)*/
        SecByteBlock &key, byte *ivBytes);
void decryptFile(const string &inputFileName, const string &outputFileName, 
        const string &authInputFileName, SecByteBlock &key, byte *ivBytes);
// Message Authentication Codes
void createMacForFile(const string &inputFileName, const string &outputFileName);
void verifyMacForFile(const string &macFileName, const string &macInputFileName);

#endif