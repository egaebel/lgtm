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
 * Runs through all the images to find the maximum width of an image and
 * the maximum height of an image.
 * Runs through all the images again to pad all the images to meet the maximum
 * height and maximum width. 
 * Padding is done such that the images are centered.
 *
 * This code was adapted from OpenCV tutorials found at:
 */
#include "pad_images.hpp"

using namespace cv;
using namespace std;

/// Global Variables
static Mat src, dst;
static int top_border, bottom_border, left_border, right_border;
static const char* windowName = "copyMakeBorder Demo";
static string csvFileName = "../pad-images/yalefaces.csv";
static const char separator = ';';

// Function headers
static int makeBorder(string fileName, int newImageHeight, int newImageWidth);

void padImages(string csvFileName) {
    // Find max sizes
    int maxHeight = 0;
    int maxWidth = 0;
    Mat sizeMat;
    std::ifstream file(csvFileName.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given fileName.";
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
                return;
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
    std::ifstream file2(csvFileName.c_str(), ifstream::in);
    if (!file2) {
        string error_message = "No valid input file was given, please check the given fileName.";
        CV_Error(CV_StsBadArg, error_message);
    }
    while (getline(file2, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(!path.empty()) {
            if (makeBorder(path, maxHeight, maxWidth) != 0) {
                cout << "There was an error in makeBorder!!" << endl;
                return;
            }
        }
    }

    // Check sizes again....
    // This is a sanity check to ensure that there were no rounding errors in padding, etc.
    int minHeight = INT_MAX;
    int minWidth = INT_MAX;
    std::ifstream file3(csvFileName.c_str(), ifstream::in);
    if (!file3) {
        string error_message = "No valid input file was given, please check the given fileName.";
        CV_Error(CV_StsBadArg, error_message);
    }
    while (getline(file3, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        if(!path.empty()) {
            sizeMat = imread(path);
            if(sizeMat.empty()) {
                printf(" No data entered, please enter the path to an image file \n");
                return;
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

    // These should be the same!
    printf("MaxHeight: %d\n", maxHeight);
    printf("MaxWidth: %d\n", maxWidth);

    printf("\nMinHeight: %d\n", minHeight);
    printf("MinWidth: %d\n", minWidth);
}

static int makeBorder(string fileName, int newImageHeight, int newImageWidth) {
    /// Load an image
    src = imread(fileName.c_str());

    if(src.empty()) {
        printf(" No data entered, please enter the path to an image file \n");
        return -1;
    }

    // Create window
    namedWindow(windowName, WINDOW_AUTOSIZE);

    // Initialize arguments for the filter
    top_border = floor((newImageHeight - src.size().height) / 2.0); 
    bottom_border = ceil((newImageHeight - src.size().height) / 2.0);
    left_border = floor((newImageWidth - src.size().width) / 2.0); 
    right_border = ceil((newImageWidth - src.size().width) / 2.0);
    dst = src;

    // Show original
    imshow(windowName, dst);

    // Show padded and wait for user acceptance or rejection
    int c;
    for(;;) {
        Scalar value(255, 255, 255);
        copyMakeBorder(src, dst, top_border, bottom_border, left_border, right_border, 
                BORDER_CONSTANT, value);

        imshow(windowName, dst);

        int keycode = waitKey(0);
        // Space bar to accept
        if (keycode == 32) {
            printf("Accepting Change!!\n");
            printf("Writing dst to: %s\n", fileName.c_str());
            imwrite(fileName, dst);
            break;
        // Escape key to reject (Meaning the padded image is not saved)
        } else if (keycode == 27) {
            printf("Rejecting Change!!\n");
            break;       
        }

    }
    return 0;
}

// Test code for padding a single image
/* 
static string test_image_fileName = "../data/me/2015-12-20-150900.jpg";
static RNG rng(12345);
static int borderType;

int main(void) {
    string test_image_fileName = "../data/me/2015-12-20-150900.jpg";
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
