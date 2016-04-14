#ifndef LGTM_CRYPTO_RUNNER_H_
#define LGTM_CRYPTO_RUNNER_H_

#include "lgtm_crypto.hpp"
#include "lgtm_file_utils.hpp"

#include "../../cryptopp/filters.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using CryptoPP::FileSink;
using CryptoPP::FileSource;
using CryptoPP::SecByteBlock;

using std::ofstream;
using std::runtime_error;
using std::vector;

void firstMessage();
bool replyToFirstMessage();
bool secondMessage();
bool replyToSecondMessage();
bool thirdMessage();
bool replyToThirdMessage();
bool decryptThirdMessageReply();

#endif