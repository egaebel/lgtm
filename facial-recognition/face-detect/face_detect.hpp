#ifndef FACE_DETECT_HPP_
#define FACE_DETECT_HPP_

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>

// Function Headers
void cropImagesToFaces(std::string csvFileName);

#endif