#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>

using namespace std;
using namespace cv;

// Function Headers
Mat detectAndDisplay(Mat frame);

// Global variables
// Copy this file from opencv/data/haarscascades to target folder
string face_cascade_name = "/home/egaebel/Programs/openCV/opencv/data/haarcascades/haarcascade_frontalface_default.xml";
string csv_filename = "test.csv";
char separator = ';';
CascadeClassifier face_cascade;
string window_name = "Capture - Face detection";
int filenumber; // Number of file to be saved
string filename;

// Function main
int main(void)
{
    // Load the cascade
    if (!face_cascade.load(face_cascade_name)){
        printf("--(!)Error loading\n");
        return (-1);
    }

    // Read CSV file
    std::ifstream file(csv_filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    
    string line, path;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        printf("path: %s\n", path.c_str());
        if(!path.empty()) {
            // Single image, test path setting
            // path = "../data/me/2015-12-21-173149.jpg";
            // Read the image file
            Mat frame = imread(path);

            // Apply the classifier to the frame
            if (!frame.empty()){
                Mat cropped_image = detectAndDisplay(frame);
                while (true) {
                    int keycode = waitKey(0);
                    // Space bar to accept
                    //if (keycode == 32) {
                    if (keycode == 1048608) {
                        printf("Accepting Change!!\n");
                        imwrite(path, cropped_image);
                        break;
                    // Escape key to reject (really just don't do anything)
					//} else if (keycode == 27) {
                    } else if (keycode == 1048603) {
                        printf("Rejecting Change!!\n");
                        break;       
                    } else {
						printf("Oops, wrong key, %d, press Space to accept or Escape to reject!\n", keycode);
					}
                }
            } else{
                printf(" --(!) No captured frame -- Break!\n");
                break;
            }
        }
    }

    return 0;
}

// Function detectAndDisplay
Mat detectAndDisplay(Mat frame) {
    std::vector<Rect> faces;
    Mat frame_gray;
    Mat crop;
    Mat res;
    Mat gray;
    string text;
    stringstream sstm;

    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    // Detect faces
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

    // Set Region of Interest
    cv::Rect roi_b;
    cv::Rect roi_c;

    size_t ic = 0; // ic is index of current element
    int ac = 0; // ac is area of current element

    size_t ib = 0; // ib is index of biggest element
    int ab = 0; // ab is area of biggest element

    for (ic = 0; ic < faces.size(); ic++) // Iterate through all current elements (detected faces)
    {
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

        if (ac > ab)
        {
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

        // Form a filename
        filename = "";
        stringstream ssfn;
        ssfn << filenumber << ".png";
        filename = ssfn.str();
        filenumber++;

        imwrite(filename, gray);

        // Display detected faces on main window - live stream from camera
        Point pt1(faces[ic].x, faces[ic].y); 
        Point pt2((faces[ic].x + faces[ic].height), (faces[ic].y + faces[ic].width));
        rectangle(frame, pt1, pt2, Scalar(0, 255, 0), 2, 8, 0);
    }

    // Show image
    sstm << "Crop area size: " << roi_b.width << "x" << roi_b.height << " Filename: " << filename;
    text = sstm.str();

    putText(frame, text, cvPoint(30, 30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0, 0, 255), 1, CV_AA);
    imshow("original", frame);

    if (!crop.empty()) {
        imshow("detected", crop);
    } else {
        printf("Not detected, destroying....\n");
        destroyWindow("detected");
    }

    return crop;
}
