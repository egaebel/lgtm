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

#ifndef LGTM_FILE_UTILS_HPP_
#define LGTM_FILE_UTILS_HPP_

#include "../../cryptopp/filters.h"
#include "../../cryptopp/hex.h"
#include "../../cryptopp/secblock.h"


#include <fstream>
#include <iostream>
#include <stdexcept>

using CryptoPP::HexEncoder;
using CryptoPP::SecByteBlock;
using CryptoPP::StringSource;
using CryptoPP::StringSink;

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::vector;

//~Function Headers---------------------------------------------------------------------------------
void readFromFile(const string &fileName, SecByteBlock &input);
void writeToFile(const string &fileName, SecByteBlock &output);
void combineFiles(const vector<string> &inputFileNames, const string &outputFileName);
void splitFile(const string &inputFileName, const vector<string> &outputFileNames,
        const vector<int> &bytesPerFile);
void printFile(const string &fileName);
void printFiles(const vector<string> &fileNames);
#endif