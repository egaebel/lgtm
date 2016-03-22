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

// Switch between Local Binary Patterns Histograms(2), Eigenfaces (1), and Fisherfaces (0)
#define FACIAL_RECOGNITION_MODEL 2

#include <opencv2/core/core.hpp>
#include <opencv2/face.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <chrono>
#include <ctime>

using namespace cv;
using namespace std;

/**
 * Read images and labels from the csv file specified by fileName.
 */
static void read_csv(const string& fileName, vector<Mat>& images, vector<int>& labels, 
        string filePrefix="", char separator=';') {
    std::ifstream file(fileName.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given fileName.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    Mat readImage;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        // If we have a prefix specified and it wasn't found then ignore this sample
        size_t foundPrefix = path.find(filePrefix);
        if (!filePrefix.empty() && foundPrefix == std::string::npos) {
            continue;
        }
        if(!path.empty() && !classlabel.empty()) {
            cout << "Reading image: " << path << endl;
            readImage = imread(path, 0);
            if (readImage.empty()) {
                cerr << "Error opening image file at path: \"" << path << endl;
                continue;
            }
            images.push_back(readImage);
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}

int main(int argc, const char *argv[]) {

    // Get the path to your CSV:
    if (argc != 2) {
        cout << "Wrong number of args! Usage is train_classifier <csv filename>" << endl;
        exit(1);
    }
    // string csvFileName = "yalefaces.csv";
    string csvFileName(argv[1]);
    cout << "Using filename: " << csvFileName << endl;

    // These vectors hold the images and corresponding labels:
    vector<Mat> images;
    vector<int> labels;

    // Read in the data (fails if no valid input fileName is given, but you'll get an error message):
    try {
        // Loop over all files in directory
        read_csv(csvFileName, images, labels);
    } catch (cv::Exception& e) {
        cerr << "Error opening file \"" << csvFileName << "\". Reason: " << e.msg << endl;
        // nothing more we can do
        exit(1);
    }

    // Get the height from the first image. We'll need this
    // later in code to reshape the images to their original
    // size AND we need to reshape incoming faces to this size:
    int imgWidth = images[0].cols;
    int imgHeight = images[0].rows;

    // Create a FaceRecognizer and train it on the given images:
    Ptr<face::FaceRecognizer> model;
    switch (FACIAL_RECOGNITION_MODEL) {
        case 0:
            cout << "Using Fisherfaces" << endl;
            model = face::createFisherFaceRecognizer();
            break;
        case 1:
            cout << "Using Eigenfaces" << endl;
            model = face::createEigenFaceRecognizer();
            break;
        case 2:
            cout << "Using Local Binary Pattern Histograms" << endl;
            int radius = 10;
            int neighbors = 8;
            int gridX = 4;
            int gridY = 4;
            double threshold = 20.0;
            cout << "Threshold is: " << threshold << endl;
            model = face::createLBPHFaceRecognizer(radius, neighbors, gridX, gridY, threshold);
            break;
    }
    cout << "starting training..." << endl;

    // Time training...
    chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    
    model->train(images, labels);
    model->save("facial-recognition-model");

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    cout << "Done training! Took " << elapsed_seconds.count() << " seconds" << endl;

    return 0;
}
