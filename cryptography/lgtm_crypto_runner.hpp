#ifndef LGTM_CRYPTO_RUNNER_H_
#define LGTM_CRYPTO_RUNNER_H_

#include "lgtm_crypto.hpp"

#include "../../cryptopp/filters.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using CryptoPP::FileSink;
using CryptoPP::FileSource;

using std::ofstream;
using std::runtime_error;
using std::vector;

void firstMessage();
void replyToFirstMessage();
void secondMessage();
void replyToSecondMessage();
void thirdMessage();
void replyToThirdMessage();
void decryptThirdMessageReply();

#endif