// /home/egaebel/Programs/openCV/opencv-3.0.0/data/haarcascades/haarcascade_frontalface_default.xml

/*
 * Copyright (c) 2011. Philipp Wagner <bytefish[at]gmx[dot]de>.
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 */

// Switch between Eigenfaces (1) and Fisherfaces (0)
#define EIGENFACES 0

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
        char separator=';') {
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
        if(!path.empty() && !classlabel.empty()) {
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
    string csvFileName = "yalefaces.csv";

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
    if (EIGENFACES) {
        cout << "Using Eigenfaces" << endl;
        model = face::createEigenFaceRecognizer();
    } else {
        cout << "Using Fisherfaces" << endl;
        model = face::createFisherFaceRecognizer();
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