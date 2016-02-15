/**
 * @file copyMakeBorder_demo.cpp
 * @brief Sample code that shows the functionality of copyMakeBorder
 * @author OpenCV team
 */

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdlib.h>
#include <stdio.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace std;

/// Global Variables
static Mat src, dst;
static int top_border, bottom_border, left_border, right_border;
static int borderType;
static const char* window_name = "copyMakeBorder Demo";
static RNG rng(12345);
static string test_image_filename = "../data/me/2015-12-20-150900.jpg";
static string csv_filename = "test.csv";
static const char separator = ';';

// Function headers
int makeBorder(string filename, int newImageHeight, int newImageWidth);

/**
 * @function main
 */
int main(int argc, char** argv) {

    // Find max sizes
    int maxHeight = 0;
    int maxWidth = 0;
    Mat sizeMat;
    std::ifstream file(csv_filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(!path.empty()) {
            sizeMat = imread(path);
            if(sizeMat.empty()) {
                printf(" No data entered, please enter the path to an image file \n");
                return -1;
            }
            Size size = sizeMat.size();
            if (size.width > maxWidth) {
                maxWidth = size.width;
            }
            if (size.height > maxHeight) {
                maxHeight = size.height;
            }
        }
    }
    printf("MaxHeight: %d\n", maxHeight);
    printf("MaxWidth: %d\n", maxWidth);


    // Draw borders
    std::ifstream file2(csv_filename.c_str(), ifstream::in);
    if (!file2) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    while (getline(file2, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(!path.empty()) {
            if (makeBorder(path, maxHeight, maxWidth) != 0) {
                return -1;
            }
        }
    }


    // Check sizes again....
    int minHeight = 9999;
    int minWidth = 9999;
    std::ifstream file3(csv_filename.c_str(), ifstream::in);
    if (!file3) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    while (getline(file3, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(!path.empty()) {
            sizeMat = imread(path);
            if(sizeMat.empty()) {
                printf(" No data entered, please enter the path to an image file \n");
                return -1;
            }
            Size size = sizeMat.size();
            if (size.width < minWidth) {
                minWidth = size.width;
            }
            if (size.height < minHeight) {
                minHeight = size.height;
            }
        }
    }

    printf("MaxHeight: %d\n", maxHeight);
    printf("MaxWidth: %d\n", maxWidth);

    printf("\nMinHeight: %d\n", minHeight);
    printf("MinWidth: %d\n", minWidth);

    return 0;
}

int makeBorder(string filename, int newImageHeight, int newImageWidth) {
    /// Load an image
    src = imread(filename.c_str());

    if(src.empty()) {
        printf(" No data entered, please enter the path to an image file \n");
        return -1;
    }

    /// Create window
    namedWindow(window_name, WINDOW_AUTOSIZE);

    /// Initialize arguments for the filter
    top_border = floor((newImageHeight - src.size().height) / 2.0); 
    bottom_border = ceil((newImageHeight - src.size().height) / 2.0);
    left_border = floor((newImageWidth - src.size().width) / 2.0); 
    right_border = ceil((newImageWidth - src.size().width) / 2.0);
    dst = src;

    imshow(window_name, dst);

    int c;
    for(;;) {
        Scalar value(255, 255, 255);
        copyMakeBorder(src, dst, top_border, bottom_border, left_border, right_border, BORDER_CONSTANT, value);

        imshow(window_name, dst);

        int keycode = waitKey(0);
        // Space bar to accept
        if (keycode == 32) {
            printf("Accepting Change!!\n");
            printf("Writing dst to: %s\n", filename.c_str());
            imwrite(filename, dst);
            break;
        // Escape key to reject (really just don't do anything)
        } else if (keycode == 27) {
            printf("Rejecting Change!!\n");
            break;       
        }

    }
    return 0;
}






/*
int main(void) {
    string test_image_filename = "../data/me/2015-12-20-150900.jpg";
    // let border be the same in all directions
    int border = 2;
    // constructs a larger image to fit both the image and the border
    Mat gray_buf(rgb.rows + 2 * border, rgb.cols + 2 * border, rgb.depth());
    // select the middle part of it w/o copying data
    Mat gray(gray_canvas, Rect(border, border, rgb.cols, rgb.rows));
    // convert image from RGB to grayscale
    cvtColor(rgb, gray, COLOR_RGB2GRAY);
    // form a border in-place
    copyMakeBorder(gray, gray_buf, border, border,
                   border, border, BORDER_REPLICATE);

    imshow(bordered_image);
    return 0;
}
*/