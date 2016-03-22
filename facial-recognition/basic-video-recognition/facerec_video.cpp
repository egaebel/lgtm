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
 * The license below covers the code in int main as well as the readCsv function.
 *
 * The prior MIT license covers any modifications to that code chunk and all of
 * the code besides the chunks indicated above.
 */ 

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

#include <cmath>

using namespace cv;
using namespace std;

// Function Headers---------------------------------------------------------------------------------
static void drawTargettingLines(int &frameWidth, int &frameHeight, Mat &frame);
static void findAngleBounds(int &frameWidth, int &frameHeight, Rect &faceRectangle, 
        double &leftSideAngle, double &rightSideAngle, double &topAngle, double &bottomAngle);

/**
 * Read images and labels from the csv file specified by fileName.
 */
static void readCsv(const string& fileName, vector<Mat>& images, vector<int>& labels, 
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
    // Check for valid command line arguments, print usage
    // if no arguments were given.
    if (argc < 4) {
        cout << "usage: " << argv[0] 
                << " </path/to/haarCascade> </path/to/csv.ext> </path/to/device id>" << endl;
        cout << "\t </path/to/haarCascade> -- Path to the Haar Cascade for face detection." 
                << endl;
        cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
        cout << "\t <device id> -- The webcam device id to grab frames from." << endl;
        exit(1);
    }

    // Get the path to your CSV:
    string haarCascadeFileName = string(argv[1]);
    string csvFileName = string(argv[2]);
    int deviceId = atoi(argv[3]);
    string trainedClassifierPath;
    if (argc == 5) {
        cout << "Reading trainedClassifierPath from input...." << endl;
        trainedClassifierPath = string(argv[4]);
        cout << "Read trainedClassifierPath as: " << trainedClassifierPath << endl;
    }

    // These vectors hold the images and corresponding labels:
    vector<Mat> images;
    vector<int> labels;

    // If we have loaded a classifier there will be no training, so no files are needed.
    if (trainedClassifierPath.empty()) {
        // Read in the data (fails if no valid input fileName is given, 
        // but you'll get an error message):
        try {
            // Loop over all files in directory
            readCsv(csvFileName, images, labels);
        } catch (cv::Exception& e) {
            cerr << "Error opening file \"" << csvFileName << "\". Reason: " << e.msg << endl;
            // nothing more we can do
            exit(1);
        }
    }

    int imgWidth = 168;
    int imgHeight = 168;

    // Create a FaceRecognizer and train it on the given images:
    Ptr<face::FaceRecognizer> model;

    switch (FACIAL_RECOGNITION_MODEL) {
        case 0:
            {
                cout << "Using Fisherfaces" << endl;
                double threshold = 2200.0;
                model = face::createFisherFaceRecognizer(/*0, threshold*/);
            }
            break;
        case 1:
            {
                cout << "Using Eigenfaces" << endl;
                double threshold = 7250.0;
                model = face::createEigenFaceRecognizer(/*0, threshold*/);
            }
            break;
        case 2:
            {
                cout << "Using Local Binary Pattern Histograms" << endl;
                int radius = 10;
                int neighbors = 8;
                int gridX = 4;
                int gridY = 4;
                double threshold = 25.0;
                cout << "Threshold is: " << threshold << endl;
                model = face::createLBPHFaceRecognizer(radius, neighbors, gridX, gridY, threshold);
            }
            break;
    }

    // Load model if a path to a pretrained model was passed
    if (trainedClassifierPath.empty())  {
        cout << "Starting training..." << endl;
        // Time training...
        chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        
        model->train(images, labels);
        model->save("new-facial-recognition-model");

        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        cout << "Done training! Took " << elapsed_seconds.count() << " seconds" << endl;
    } else {
        cout << "Starting loading from file: " << trainedClassifierPath << endl;
        chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        
        model->load(trainedClassifierPath);

        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        cout << "Done loading classifier from: " << trainedClassifierPath 
                << " ! Took " << elapsed_seconds.count() << " seconds" << endl;
    }

    // That's it for learning the Face Recognition model. You now
    // need to create the classifier for the task of Face Detection.
    // We are going to use the haar cascade you have specified in the
    // command line arguments:
    CascadeClassifier haarCascade;
    haarCascade.load(haarCascadeFileName);
    
    // Get a handle to the Video device:
    VideoCapture cap(deviceId);
    // Check if we can use this device at all:
    if(!cap.isOpened()) {
        cerr << "Capture Device ID " << deviceId << "cannot be opened." << endl;
        return -1;
    }
    int capFrameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int capFrameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    try {
        // Holds the current frame from the Video device:
        Mat frame;
        for(;;) {
            cap >> frame;
            // Clone the current frame:
            Mat original = frame.clone();
            // Convert the current frame to grayscale:
            Mat gray;
            cvtColor(original, gray, CV_BGR2GRAY);
            // Find the faces in the frame:
            vector< Rect_<int> > faces;
            haarCascade.detectMultiScale(gray, faces);
            // At this point you have the position of the faces in
            // faces. Now we'll get the faces, make a prediction and
            // annotate it in the video. Cool or what?
            for(int i = 0; i < faces.size(); i++) {
                // Process face by face:
                Rect curFace = faces[i];
                // Crop the face from the image. So simple with OpenCV C++:
                Mat face = gray(curFace);
                // Resizing the face is necessary for Eigenfaces and Fisherfaces. You can easily
                // verify this, by reading through the face recognition tutorial coming with OpenCV.
                // Resizing IS NOT NEEDED for Local Binary Patterns Histograms, so preparing the
                // input data really depends on the algorithm used.
                //
                // I strongly encourage you to play around with the algorithms. See which work best
                // in your scenario, LBPH should always be a contender for robust face recognition.
                //
                // Since I am showing the Fisherfaces algorithm here, I also show how to resize the
                // face you have just found:
                Mat resizedFace;
                cv::resize(face, resizedFace, Size(imgWidth, imgHeight), 1.0, 1.0, INTER_CUBIC);
                // Now perform the prediction, see how easy that is:
                int prediction = -1;
                double confidence = 0.0;
                model->predict(resizedFace, prediction, confidence);
                // If the prediction is one of the faces we can recognize
                if (prediction > 0) {
                    // And finally write all we've found out to the original image!
                    // First of all draw a green rectangle around the detected face:
                    rectangle(original, curFace, CV_RGB(0, 255,0), 1);
                    // Create the text we will annotate the box with:
                    string boxPredictionText = format("Prediction = %d", prediction);
                    string boxConfidenceText = format("With confidence: %g", confidence);
                    // Calculate the position for annotated text (make sure we don't
                    // put illegal values in there):
                    // TODO: See below, 10 was the original
                    int predictionPosX = std::max(curFace.tl().x - 10, 0);
                    int predictionPosY = std::max(curFace.tl().y - 25, 0);
                    int confidencePosX = std::max(curFace.tl().x - 10, 0);
                    int confidencePosY = std::max(curFace.tl().y - 10, 0);
                    // And now put it into the image:
                    putText(original, boxPredictionText, Point(predictionPosX, predictionPosY), 
                            FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
                    putText(original, boxConfidenceText, Point(confidencePosX, confidencePosY), 
                            FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
                    double leftSideAngle = -1;
                    double rightSideAngle = -1;
                    double topAngle = -1;
                    double bottomAngle = -1;
                    findAngleBounds(capFrameWidth, capFrameHeight, curFace, 
                            leftSideAngle, rightSideAngle, 
                            topAngle, bottomAngle);
                    int thetaPosX = std::max(curFace.tl().x + 10, 0);
                    int thetaPosY = std::max(curFace.br().y + 10, 0);
                    int phiPosX = std::max(curFace.tl().x + 10, 0);
                    int phiPosY = std::max(curFace.br().y + 25, 0);
                    string boxThetaText = format("Theta 1: %g, Theta2: %g", 
                            leftSideAngle, rightSideAngle);
                    string boxPhiText = format("Phi1: %g, Phi2: %g", topAngle, bottomAngle);
                    putText(original, boxThetaText, Point(thetaPosX, thetaPosY), 
                            FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
                    putText(original, boxPhiText, Point(phiPosX, phiPosY), 
                            FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
                    int xPosX = std::max(curFace.tl().x + 10, 0);
                    int xPosY = std::max(curFace.br().y + 40, 0);
                    string xText = format("Left side X position: %d", 
                            (curFace.tl().x - capFrameWidth / 2));
                    putText(original, xText, Point(xPosX, xPosY), FONT_HERSHEY_PLAIN, 1.0, 
                        CV_RGB(0, 255, 0), 2.0);
                }
            }
            // Add "targeting" lines
            drawTargettingLines(capFrameWidth, capFrameHeight, original);

            // Show the result:
            imshow("face_recognizer", original);
            // And display it:
            int key = waitKey(20);
            // Exit this loop on escape OR space:
            if (key == 27 || key == 32) {
                break;
            } else if (key != -1) {
                cout << "Keydown was: " << key 
                        << " Need to press esc or space to exit loop!" << endl;
            }
        }
    } catch(Exception e) {
        cap.release();
    }
    cap.release();
    return 0;
}

/**
 * Wraps the passed angle to the range [-pi / 2, pi / 2]
 */
static double wrapAngle(double angle) {
    return angle - M_PI * floor(angle / M_PI) - (M_PI / 2);
}

/**
 * Calculates the horizontal angles and vertical angles of the passed in rectangle.
 * Annotates the rectangle on the frame with text describing the output.
 */
static void findAngleBounds(int &frameWidth, int &frameHeight, Rect &faceRectangle, 
        double &leftSideAngle, double &rightSideAngle, double &topAngle, double &bottomAngle) {
    // Get points at the two bottom corners of the rectangle
    int x1 = faceRectangle.tl().x - frameWidth / 2;
    int x2 = faceRectangle.br().x - frameWidth / 2;

    // Compute theta1 and theta2 (leftSideAngle and rightSideAngle)
    // Convert to polar coordinates, with known r and unknown angle
    double r = 639;
    leftSideAngle = acos(x1 / r);
    rightSideAngle = acos(x2 / r);
    // Wrap angles to [-90, 90]
    leftSideAngle = wrapAngle(leftSideAngle) * (180 / M_PI);
    rightSideAngle = wrapAngle(rightSideAngle) * (180 / M_PI);
    // Invert the circle revolution direction
    leftSideAngle *= -1;
    rightSideAngle *= -1;
}

/**
 * Draw a straight horizontal line across the center of the frame and annotate it with 
 * smaller vertical lines which have angle annotations from -30 - 30
 */ 
static void drawTargettingLines(int &frameWidth, int &frameHeight, Mat &frame) {
    // Straight horizontal line across the center of the frame
    line(frame, Point(0, frameHeight / 2), Point(frameWidth, frameHeight / 2), CV_RGB(0, 255,0), 1);
    int topY = frameHeight / 2 - 50;
    int bottomY = frameHeight / 2 + 50;
    double x1, x2;
    double r = 639;
    double theta;
    string angleString1;
    string angleString2;
    for (int i = 0; i <= 30; i+=10) {
        x1 = r * sin(i * (M_PI / 180));
        x1 += frameWidth / 2;        
        x2 = r * sin(-i * (M_PI / 180));
        x2 += frameWidth / 2;
        line(frame, Point(x1, topY), Point(x1, bottomY), CV_RGB(0, 255,0), 1);
        line(frame, Point(x2, topY), Point(x2, bottomY), CV_RGB(0, 255,0), 1);
        angleString1 = format("%d", i);
        angleString2 = format("%d", -i);
        if (i != 30) {
            putText(frame, angleString1, Point(x1 - 5, bottomY + 15), FONT_HERSHEY_PLAIN, 1.0, 
                    CV_RGB(0, 255, 0), 2.0);    
            if (i != 0) {
                putText(frame, angleString2, Point(x2 - 20, bottomY + 15), FONT_HERSHEY_PLAIN, 1.0, 
                        CV_RGB(0, 255, 0), 2.0);    
            }
        } else {
            putText(frame, angleString1, Point(x1 - 20, bottomY + 15), FONT_HERSHEY_PLAIN, 1.0, 
                    CV_RGB(0, 255, 0), 2.0);    
            putText(frame, angleString2, Point(x2 - 5, bottomY + 15), FONT_HERSHEY_PLAIN, 1.0, 
                    CV_RGB(0, 255, 0), 2.0);    
        }
    }
}