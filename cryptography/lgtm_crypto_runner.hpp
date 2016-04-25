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