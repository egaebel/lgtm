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

/**
 * Takes a csv file specified in the variable "csvFileName" where each line indicates an 
 * image file and a label.
 * Runs through all the images and augments them in various ways (flips, rotations, and both)
 * and saves the altered images to the directory the image was retrieved from.
 * This is useful to increase the size of the training set.
 */
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <climits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
 
#define DEBUG 0

using namespace cv;
using namespace std;

/// Global Variables
static Mat src, dst;
static int top_border, bottom_border, left_border, right_border;
static string csvFileName = "yalefaces.csv";
static const char separator = ';';
static const int nThreads = 4;
static const string threadText = "__thread--";
static pthread_mutex_t fileStringMutex;

// Function headers
static void* augmentDataInThread(void* fileName);
static void rotateImage(const Mat &input, Mat &output, double alpha, double beta, double gamma, 
        double dx, double dy, double dz, double f);

// Run, run, run
int main(void) {
    // Find max sizes
    int maxHeight = 0;
    int maxWidth = 0;
    std::ifstream file(csvFileName.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given fileName.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line;

    int numLines = std::count(std::istreambuf_iterator<char>(file), 
            std::istreambuf_iterator<char>(), '\n');
    // Loop over successive chunks of the original file and create new files with each chunk
    for (int i = 0, startPos = 0; i < nThreads; i++, startPos += (numLines / nThreads)) {
        std::ofstream choppedFile(csvFileName + threadText + to_string(i));
        // Seek to the correct starting point
        file.seekg(0);
        for (int j = 0; j < startPos; j++) {
            getline(file, line);
        }
        // Get all chunk of lines from file, if on the last iteration go to the end of the file
        int j = 0;
        while (j < (numLines / nThreads) 
                || (i == (nThreads - 1) && !file.eof())) {
            getline(file, line);
            choppedFile << line;
            choppedFile << "\n";
            j++;
        }
        choppedFile.close();
    }
    file.close();

    // Generate file names prior to thread creation to avoid races
    vector<string> threadFileNames;
    for (int i = 0; i < nThreads; i++) {
        string tempFileName = csvFileName + threadText + to_string(i);
        threadFileNames.push_back(string(tempFileName));
    }
    if (pthread_mutex_init(&fileStringMutex, NULL) != 0) {
        cout << "Error in pthread_mutex_init" << endl;
    }

    // Process in nThreads
    pthread_t threads[nThreads];
    for (int i = 0; i < nThreads; i++) {
        // cv::namedWindow("Z-Rotation: Data Augmentation");
        if (pthread_create(&threads[i], NULL, augmentDataInThread, (void*) threadFileNames[i].c_str()) != 0) {
            cout << "Error in pthread_create" << endl;
        }
    }

    for (int i = 0; i < nThreads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            cout << "Error in pthread_join on thread: " << threads[i] << " at index " << i << endl;
        }
    }
    if (pthread_mutex_destroy(&fileStringMutex) != 0) {
        cout << "Error in pthread_mutex_destroy" << endl;
    }
    return 0;
}

static void* augmentDataInThread(void* fileName) {
    string stringFileName((char*) fileName);
    pthread_mutex_lock(&fileStringMutex);
    cout << "augmentDataInThread, fileName: ||" << stringFileName.c_str() << "||" << endl;
    pthread_mutex_unlock(&fileStringMutex);
    std::ifstream file(stringFileName.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given fileName: "
                + stringFileName;
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, rotationFileName;
    Mat imgSrc;
    Mat imgRotation;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(path.empty()) {
            continue;
        }
        
        imgSrc = imread(path);

        // Loop over every degree of rotation on all three axes, save results
        int zAxisRotation = -5;
        while (zAxisRotation <= 5) {
            int xAxisRotation = -5;
            while (xAxisRotation <= 5) {
                int yAxisRotation = -5;
                while (yAxisRotation <= 5) {;
                    // x-axis rotation, empirical range: [-12.5, 12.5]
                    double alpha = xAxisRotation;
                    // y-axis rotation, empirical range: [-10, 10]
                    double beta = yAxisRotation;
                    // z-axis rotation, range: [0, 360)
                    double gamma = zAxisRotation;
                    double dz = 200;
                    double focalDistance = 200;
                    rotateImage(imgSrc, imgRotation, alpha, beta, gamma, 0, 0, dz, focalDistance);
                    //imshow("Z-Rotation: Data Augmentation", imgRotation);
                    //waitKey(20);
                    //sleep(1.5);
                    // Make file name for augmented data
                    rotationFileName = path.substr(0, path.size() - 4) 
                            + "--rotated--z-" + std::to_string(zAxisRotation) 
                            + "--y-" + std::to_string(yAxisRotation) 
                            + "--x-" + std::to_string(xAxisRotation) 
                            + "--" + path.substr(path.size() - 4, path.size());
                    imwrite(rotationFileName, imgRotation);
                    yAxisRotation += 5;
                }
                xAxisRotation += 5;
            }
            zAxisRotation += 5;
        }
    }
    file.close();
    pthread_exit(NULL);
}

/**
 *
 */
static void rotateImage(const Mat &input, Mat &output, double alpha, double beta, double gamma, 
        double dx, double dy, double dz, double f) {
    //alpha = (alpha - 90.) * CV_PI / 180.;
    //beta = (beta - 90.) * CV_PI / 180.;
    //gamma = (gamma - 90.) * CV_PI / 180.;
    alpha = alpha * CV_PI / 180.;
    beta = beta * CV_PI / 180.;
    gamma = gamma * CV_PI / 180.;
    // get width and height for ease of use in matrices
    double w = (double) input.cols;
    double h = (double) input.rows;
    // Projection 2D -> 3D matrix
    Mat A1 = (Mat_<double>(4,3) <<
              1, 0, -w/2,
              0, 1, -h/2,
              0, 0,    0,
              0, 0,    1);
    // Rotation matrices around the X, Y, and Z axis
    Mat RX = (Mat_<double>(4, 4) <<
              1,          0,           0, 0,
              0, cos(alpha), -sin(alpha), 0,
              0, sin(alpha),  cos(alpha), 0,
              0,          0,           0, 1);
    Mat RY = (Mat_<double>(4, 4) <<
              cos(beta), 0, -sin(beta), 0,
              0, 1,          0, 0,
              sin(beta), 0,  cos(beta), 0,
              0, 0,          0, 1);
    Mat RZ = (Mat_<double>(4, 4) <<
              cos(gamma), -sin(gamma), 0, 0,
              sin(gamma),  cos(gamma), 0, 0,
              0,          0,           1, 0,
              0,          0,           0, 1);
    // Composed rotation matrix with (RX, RY, RZ)
    Mat R = RX * RY * RZ;
    // Translation matrix
    Mat T = (Mat_<double>(4, 4) <<
             1, 0, 0, dx,
             0, 1, 0, dy,
             0, 0, 1, dz,
             0, 0, 0, 1);
    // 3D -> 2D matrix
    Mat A2 = (Mat_<double>(3,4) <<
              f, 0, w / 2, 0,
              0, f, h / 2, 0,
              0, 0,   1,   0);
    // Final transformation matrix
    Mat trans = A2 * (T * (R * A1));
    // Apply matrix transformation
    warpPerspective(input, output, trans, input.size(), INTER_LANCZOS4);
  }