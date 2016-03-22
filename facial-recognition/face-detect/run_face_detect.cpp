#include "face_detect.hpp"

// Function main
int main(void) {
    std::string csvFileName = "yalefaces.csv";
    cropImagesToFaces(csvFileName);
    return 0;
}