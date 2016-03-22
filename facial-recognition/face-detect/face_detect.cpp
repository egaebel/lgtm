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
 * Runs through each image specified in csvFileName and run face detection on each one and running
 * a cropping operation to reduce the image to just the face. 
 * Present the results to the user, showing before and after for cropping and allow the user to
 * accept or reject each crop.
 * Save the accepted files.
 *
 * This code was adapted from OpenCV tutorials found at:
 */
#include "face_detect.hpp"

using namespace std;
using namespace cv;

// Global variables
static string faceCascadefileName = "../face-detect/haarcascades/haarcascade_frontalface_default.xml";
// static string csvFileName = "../face-detect/yalefaces.csv";
static char separator = ';';
static CascadeClassifier faceCascade;
static string windowName = "Capture - Face detection"; 

// Private function headers
static Mat detectAndDisplay(Mat frame);

void cropImagesToFaces(string csvFileName) {
    // Load the cascade
    if (!faceCascade.load(faceCascadefileName)){
        printf("--(!)Error loading\n");
        return;
    }

    // Read CSV file
    std::ifstream file(csvFileName.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given fileName.";
        CV_Error(CV_StsBadArg, error_message);
    }
    
    // Loop over files specified by the csv file in csvFileName.
    string line, path;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        printf("path: %s\n", path.c_str());
        if(!path.empty()) {

            // Read the image file
            Mat frame = imread(path);

            // Apply the classifier to the frame
            if (!frame.empty()){
                Mat croppedImage = detectAndDisplay(frame);
                while (true) {
                    int keycode = waitKey(0);
                    // Space bar to accept
                    // TODO: NOTE: The keycode for space started showing up as 1048608 for some 
                    // reason, it is likely that it will change back, for another unknown reason.
                    // Be ready for this.
                    //if (keycode == 1048608) {
                    if (keycode == 32) {
                        printf("Accepting Change!!\n");
                        imwrite(path, croppedImage);
                        break;
                    // Escape key to reject (really just don't do anything)
                    // TODO: NOTE: The keycode for esc started showing up as 1048603 for some 
                    // reason, it is likely that it will change back, for another unknown reason.
                    // Be ready for this.
                    //} else if (keycode == 1048603) {
                    } else if (keycode == 27) {
                        printf("Rejecting Change!!\n");
                        break;       
                    } else {
                        printf("Oops, wrong key, %d, press Space to accept or Escape to reject!\n", 
                                keycode);
                    }
                }
            } else{
                printf(" --(!) No captured frame -- Break!\n");
                break;
            }
        }
    }
}

// Function detectAndDisplay
static Mat detectAndDisplay(Mat frame) {
    std::vector<Rect> faces;
    Mat frame_gray;
    Mat crop;
    Mat res;
    Mat gray;
    string text;
    stringstream sstm;
    string fileName;

    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    // Detect faces
    faceCascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

    // Set Region of Interest
    cv::Rect roi_b;
    cv::Rect roi_c;

    // ic is index of current element
    size_t ic = 0;
    // ac is area of current element
    int ac = 0; 

    // ib is index of biggest element
    size_t ib = 0; 
    // ab is area of biggest element
    int ab = 0; 

    int numFiles;
    // Iterate through all current elements (detected faces)
    for (ic = 0; ic < faces.size(); ic++) {
        roi_c.x = faces[ic].x;
        roi_c.y = faces[ic].y;
        roi_c.width = (faces[ic].width);
        roi_c.height = (faces[ic].height);

        // Get the area of current element (detected face)
        ac = roi_c.width * roi_c.height; 

        roi_b.x = faces[ib].x;
        roi_b.y = faces[ib].y;
        roi_b.width = (faces[ib].width);
        roi_b.height = (faces[ib].height);

        // Get the area of biggest element, at beginning it is same as "current" element
        ab = roi_b.width * roi_b.height; 

        if (ac > ab) {
            ib = ic;
            roi_b.x = faces[ib].x;
            roi_b.y = faces[ib].y;
            roi_b.width = (faces[ib].width);
            roi_b.height = (faces[ib].height);
        }

        crop = frame(roi_b);

        // This will be needed later while saving images
        resize(crop, res, Size(128, 128), 0, 0, INTER_LINEAR); 
        // Convert cropped image to Grayscale
        cvtColor(crop, gray, CV_BGR2GRAY); 

        // Form a fileName
        fileName = "";
        stringstream ssfn;
        ssfn << numFiles << ".png";
        fileName = ssfn.str();
        numFiles++;

        imwrite(fileName, gray);

        // Display detected faces on main window - live stream from camera
        //Point pt1(faces[ic].x, faces[ic].y); 
        //Point pt2((faces[ic].x + faces[ic].height), (faces[ic].y + faces[ic].width));
        //rectangle(frame, pt1, pt2, Scalar(0, 255, 0), 2, 8, 0);
    }

    // Show image
    //sstm << "Crop area size: " << roi_b.width << "x" << roi_b.height << " fileName: " << fileName;
    //text = sstm.str();

    //putText(frame, text, cvPoint(30, 30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0, 0, 255), 
    //        1, CV_AA);
    imshow("original", frame);

    if (!crop.empty()) {
        imshow("detected", crop);
    } else {
        printf("Not detected, destroying....\n");
        destroyWindow("detected");
    }

    return crop;
}
