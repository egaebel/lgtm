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
            read_csv(csvFileName, images, labels);
        } catch (cv::Exception& e) {
            cerr << "Error opening file \"" << csvFileName << "\". Reason: " << e.msg << endl;
            // nothing more we can do
            exit(1);
        }
    }

    // Get the height from the first image. We'll need this
    // later in code to reshape the images to their original
    // size AND we need to reshape incoming faces to this size:
    // TODO: differentiate between when the csv file path is passed 
    //      and when a classifier path is passed
    int imgWidth = 168;//images[0].cols;
    int imgHeight = 168;//images[0].rows;

    // Create a FaceRecognizer and train it on the given images:
    Ptr<face::FaceRecognizer> model;
    if (EIGENFACES) {
        cout << "Using Eigenfaces" << endl;
        model = face::createEigenFaceRecognizer();
    } else {
        cout << "Using Fisherfaces" << endl;
        double threshold = 1700.0;
        model = face::createFisherFaceRecognizer(0, threshold);
    }

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
                // And finally write all we've found out to the original image!
                // First of all draw a green rectangle around the detected face:
                rectangle(original, curFace, CV_RGB(0, 255,0), 1);
                // Create the text we will annotate the box with:
                string boxPredictionText = format("Prediction = %d", prediction);
                string boxConfidenceText = format("With confidence: %g", confidence);
                // Calculate the position for annotated text (make sure we don't
                // put illegal values in there):
                // TODO: See below, 10 was the original
                int predictionPosX = std::max(curFace.tl().x - 25, 0);
                int predictionPosY = std::max(curFace.tl().y - 25, 0);
                int confidencePosX = std::max(curFace.tl().x - 10, 0);
                int confidencePosY = std::max(curFace.tl().y - 10, 0);
                // And now put it into the image:
                putText(original, boxPredictionText, Point(predictionPosX, predictionPosY), 
                        FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
                putText(original, boxConfidenceText, Point(confidencePosX, confidencePosY), 
                        FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0, 255, 0), 2.0);
            }
            // Show the result:
            imshow("face_recognizer", original);
            // And display it:
            int key = waitKey(1000);
            // Exit this loop on escape OR space:
            if(key == 27 || key == 32) {
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