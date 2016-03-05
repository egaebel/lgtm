//#include "../augment-data/augment_data.hpp"
#include "../face-detect/face_detect.hpp"
#include "../pad-images/pad_images.hpp"

using namespace std;

int main(void) {
    cout << "Running Full Preprocessing..." << endl;
    string csvFileName = "yalefaces.csv";
    cropImagesToFaces(csvFileName);
    padImages(csvFileName);
    return 0;
}