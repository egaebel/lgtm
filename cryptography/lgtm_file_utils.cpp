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

#include "lgtm_file_utils.hpp"

// 1 to print debug messages, 0 to suppress them
#define DEBUG_MESSAGES 0

//~File operation functions-------------------------------------------------------------------------
/**
 * Reads into a SecByteBlock from a file specified by fileName.
 */
void readFromFile(const string &fileName, SecByteBlock &input) {
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
    if (DEBUG_MESSAGES) {
        cout << "Read from file, read " << input.SizeInBytes() 
                << " bytes from file: " << fileName << endl;
    }
}

/**
 * Writes a SecByteBlock to a file specified by fileName.
 */
void writeToFile(const string &fileName, SecByteBlock &output) {
    if (DEBUG_MESSAGES) {
        cout << "Write to file, writing " << output.SizeInBytes() 
                << " bytes to file: " << fileName << endl;
    }
    ofstream outputStream(fileName, ios::out | ios::binary);
    outputStream.write((char*) output.BytePtr(), output.SizeInBytes());
    outputStream.close();
}

/**
 * Take a vector of file names and concatenate the file contents together 
 * (in the order given in inputFileNames) and output the result into outputFileName.
 */
void combineFiles(const vector<string> &inputFileNames, const string &outputFileName) {
    // Open output file
    ofstream outputStream(outputFileName, ios::out | ios::binary);
    if (!outputStream.is_open()) {
        throw runtime_error("Error in combineFiles, could not open outputFileName: " 
                + outputFileName);
    }

    // Loop over files in inputFileNames and read into outputFileName
    const int fileBufferSize = 512;
    byte fileBuffer[fileBufferSize];
    for (unsigned int i = 0; i < inputFileNames.size(); i++) {
        // Open current file name
        ifstream curInputStream(inputFileNames[i], ios::in | ios::binary);
        if (curInputStream.is_open()) {
            // Loop until we don't read any more bytes
            int numReadBytes = 0;
            do {
                numReadBytes = curInputStream.readsome((char*) fileBuffer, fileBufferSize);
                if (numReadBytes > 0) {
                    outputStream.write((char*) fileBuffer, numReadBytes);
                }
            } while (numReadBytes > 0);
        } else {
            throw runtime_error("Error in combineFiles, could not open inputFileName: " 
                    + inputFileNames[i]);
        }
        curInputStream.close();
    }
    outputStream.close();
}

/**
 * Take an inputFileName and split that file into the files named in 
 * the vector<string> outputFileNames. Each file size is looked up in bytesPerFile. 
 * The final file may not have a given length, making it a variable length file.
 */
void splitFile(const string &inputFileName, const vector<string> &outputFileNames,
        const vector<int> &bytesPerFile) {
    if (outputFileNames.size() != bytesPerFile.size() 
            && (outputFileNames.size() - 1) != bytesPerFile.size()) {
        throw runtime_error("Error in splitFile, bytesPerFile and outputFileNames can only differ in length by 1!");
    }
    ifstream inputStream(inputFileName, ios::in | ios::binary);
    if (!inputStream.is_open()) {
        throw runtime_error("Error in splitFile, inputFileName: " 
                + inputFileName + " could not be opened!");
    }

    // Get input file length
    inputStream.seekg(0, inputStream.end);
    int inputFileLength = inputStream.tellg();
    inputStream.seekg(0, inputStream.beg);

    // Loop over output files
    int bytesRead = 0;
    for (unsigned int i = 0; i < outputFileNames.size(); i++) {
        
        // Open new output stream
        ofstream curOutputStream(outputFileNames[i], ios::out | ios::binary);
        if (!curOutputStream.is_open()) {
            throw runtime_error("Error in splitFile, outputFileNames[i]: " 
                    + outputFileNames[i] + "could not be opened!");
        }

        // Set the number of bytes to read
        int numBytesToRead = 0;
        if (i < bytesPerFile.size()) {
            numBytesToRead = bytesPerFile[i];
        } else {
            numBytesToRead = inputFileLength - bytesRead;
        }
        byte *fileBuffer = new byte[numBytesToRead];
        int numReadBytes = inputStream.readsome((char*) fileBuffer, numBytesToRead);

        // Check that we read the right number of bytes
        if (numReadBytes != numBytesToRead) {
            cerr << "numReadBytes != numBytesToRead" 
                    << endl << "From file: " << inputFileName
                    << endl << "To file: " << outputFileNames[i]
                    << endl << "numReadBytes: " << numReadBytes
                    << endl << "numBytesToRead: " << numBytesToRead
                    << endl << endl;
            throw runtime_error("Error in splitFile, numReadBytes != numBytesToRead");
        }

        // Write to current output stream
        curOutputStream.write((char*) fileBuffer, numBytesToRead); 

        delete[] fileBuffer;

        curOutputStream.close();

        bytesRead += numBytesToRead;
    }
    inputStream.close();
}

/**
 * Mostly just a debugging function.
 * Takes a file name of a binary file and prints the contents of the file.
 */
void printFile(const string &fileName) {
    cout << "Printing File: " << fileName << endl;
    ifstream inputStream(fileName, ios::in | ios::binary);
    if (inputStream.is_open()) {
        const int bufferSize = 128;
        byte buffer[bufferSize];
        int bytesRead = 0;
        do {
            bytesRead = inputStream.readsome((char*) buffer, bufferSize);
            if (bytesRead > 0) {
                string encodedInputBytes;
                StringSource stringSource(buffer, bytesRead, true,
                    new HexEncoder(
                        new StringSink(encodedInputBytes)));
                cout << encodedInputBytes << endl;
            }
        } while (bytesRead > 0);
        inputStream.close();
    } else {
        cerr << "Error, in printFile, unable to open file: " << fileName << endl;
    }
    cout << "============File printed!============" << endl;
}

/**
 * Takes a vector of strings and calls printFile on each one.
 */
void printFiles(const vector<string> &fileNames) {
    for (unsigned int i = 0; i < fileNames.size(); i++) {
        printFile(fileNames[i]);
    }
}