/**
 * Takes a csv file specified in the variable "csv_fileName" where each line indicates an 
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

#include <climits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define DEBUG 0

using namespace cv;
using namespace std;

/// Global Variables
static Mat src, dst;
static int top_border, bottom_border, left_border, right_border;
static string csv_fileName = "yalefaces.csv";
static const char separator = ';';

// Function headers
void rotate(Mat& src, Mat& dst, double angle);

// Run, run, run
int main(void) {
    // Find max sizes
    int maxHeight = 0;
    int maxWidth = 0;
    Mat imgSrc;
    Mat imgRotation;
    Mat imgFlip;
    std::ifstream file(csv_fileName.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given fileName.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, rotationFileName, flipFileName;
    vector<int> compressionParams;
    compressionParams.push_back(CV_IMWRITE_JPEG_QUALITY);
    compressionParams.push_back(100);
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(path.empty()) {
            continue;
        }
        
        imgSrc = imread(path);

        // Perform flip with no rotation
        flip(imgSrc, imgFlip, 1);
        flipFileName = path.substr(0, path.size() - 4) + "--flipped" 
                + path.substr(path.size() - 4, path.size());
        cout << "Writing: " << flipFileName << endl;
        imwrite(flipFileName, imgFlip);

        // Loop over every degree of rotation, rotate, flip, save both results
        int rotation = 0;
        while (rotation < 360) {
            rotate(imgSrc, imgRotation, rotation);
            // Flip around y-axis
            flip(imgRotation, imgFlip, 1);
            if (DEBUG) {
                imshow("No Flip: Data Augmentation", imgRotation);
                imshow("Flip: Data Augmentation", imgFlip);
                for (;;) {
                    char key = (char) waitKey(20);
                    // Exit this loop on escape:
                    if(key == 27) {
                        break;
                    }
                }
            }
            // Make file names for augmented data
            rotationFileName = path.substr(0, path.size() - 4) + "--rotated-" 
                    + std::to_string(rotation) + "--" + path.substr(path.size() - 4, path.size());
            flipFileName = path.substr(0, path.size() - 4) + "--rotated-" 
                    + std::to_string(rotation) + "--flipped" 
                    + path.substr(path.size() - 4, path.size());
            cout << "Writing: " << rotationFileName << endl;
            imwrite(rotationFileName, imgRotation);
            cout << "Writing: " << flipFileName << endl;
            imwrite(flipFileName, imgFlip);
            rotation += 20;
        }
    }

    return 0;
}

/**
 * Takes a src and dst Mat and an angle, rotates src by angle and places it in dst.
 */
void rotate(Mat& src, Mat& dst, double angle) {
    //cout << RANDCOL << "R O T A T I N G" << endlr;
    //int len = std::max(src.cols, src.rows);
    Point2f srcCenterPoint(src.cols*0.5, src.rows*0.5);
    //Point2f pt(len/2., len/2.);
    Mat rotationMatrix = getRotationMatrix2D(srcCenterPoint, angle, 1.0);
    // Nearest is too rough, 
    warpAffine(src, dst, rotationMatrix, src.size(), INTER_CUBIC); 
}