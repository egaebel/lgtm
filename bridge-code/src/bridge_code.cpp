// C++ headers
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

// Linux headers
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

// My Headers

//~Constants----------------------------------------------------------------------------------------
static const std::string matlabOutputFile = "matlab_aoa_output.out";
static const std::string matlabCommand = (std::string) "matlab -nojvm -nodisplay -nosplash -r "
        + "\"run('/home/egaebel/grad-docs/research/thesis/csi-code/tests/spotfi_test.m'), exit\""
        + " > " 
        + matlabOutputFile;
static const std::string csiDataFile = "csi.dat";
static const std::string csiCollectionOutputFile = "csi-log-collection.out";
// TODO: BE SURE TO INCLUDE DOCUMENTATION EXPLAINING DEPENDENCIES!
static const std::string csiCollectionCommand = "unbuffer ./csi-collection.sh | tee " + csiCollectionOutputFile;
// TODO: How long does this need to be?
static const int MAX_NUM_CSI_OUTPUT_LINES = 70;

static void checkSystemValid();
static void collectCsiData();
static bool runMatlab(std::vector<double>&);
static void extractCsiDataContent();
static void runFacialRecognition();
static bool compareSignalLocToFaceLoc();


int main() {
    
    // Check system
    // checkSystemValid();

    // Receive data from other user (csi-collection.sh)
    collectCsiData();
    // Analyze signal from other user with MATLAB and get angle of arrival
    std::vector<double> angleOfArrivals;
    runMatlab(angleOfArrivals);
    // Extract data sent from other user and build facial recognition model
    // extractCsiDataContent();
    // Run facial recognition model on current video stream
    // runFacialRecognition();
    // Compare face location with AoA
    /*
    if (compareSignalLocToFaceLoc()) {
        // If AoA and face location match up, then authenticate    
    }
    */
    return 1;
}

static void checkSystemValid() {
    // Run checks, I.E. ensuring shell is available
}

static void collectCsiData() {
    using namespace std::chrono;
    // 3 minute timeout
    const int TIMEOUT = 60 * 3;
    // Run command, parse output file to determine when to kill command

    pid_t forkRetVal = fork();
    if (forkRetVal == -1) {
        std::cout << "ERROR FORKING IN collectCsiData!" << std::endl;
    } else if (forkRetVal == 0) {
        char* const args[] = {(char*) csiCollectionCommand.c_str(), (char*) "\0"};
        execv(csiCollectionCommand.c_str(), args);
    } else {
        int linesWritten = 0;
        steady_clock::time_point beginTime = steady_clock::now();
        steady_clock::time_point endTime = steady_clock::now();
        duration<int> timeElapsed = duration_cast<duration<int>>(endTime - beginTime);
        while (linesWritten < MAX_NUM_CSI_OUTPUT_LINES || timeElapsed.count() > TIMEOUT) {
            // Parse output file
            linesWritten = 0;
            std::ifstream infile(csiCollectionOutputFile);
            std::string line;
            while (std::getline(infile, line)) {
                linesWritten += 1;
            }
            infile.close();
            sleep(2);
            timeElapsed = duration_cast<duration<int>>(endTime - beginTime);
        }
        int killRetVal = kill(forkRetVal, SIGINT);
        if (killRetVal < 0) {
            // Do error processing with errno
        }
    }
}

static bool runMatlab(std::vector<double> &angleOfArrivals) {
    pid_t forkRetVal = fork();
    if (forkRetVal == -1) {
        std::cout << "ERROR FORKING IN runMatlab!" << std::endl;
        return false;
    } else if (forkRetVal == 0) {
        char* const args[] = {(char*) matlabCommand.c_str(), (char*) "\0"};
        execv(matlabCommand.c_str(), args);
        return false;
    } else {
        int status = 0;
        int waitRetVal = waitpid(forkRetVal, &status, 0);
        if (waitRetVal < 0) {
            // Do error processing
            return false;
        }
        if (status < 0) {
            // Do status processing (not necessarily an error...)
            // return NULL;
        }
        // On success, read the matlab output file
        std::ifstream infile(matlabOutputFile);
        std::string line;
        // TODO: In MATLAB file ensure that output is AoA, line-by-line, 
        // where the first AoA is the most likely
        double parsedAngleOfArrival;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            std::cout << "Read " << line << " from " << matlabOutputFile << std::endl;
            parsedAngleOfArrival = std::stod(line, NULL);
            angleOfArrivals.push_back(parsedAngleOfArrival);
        }
        infile.close();
        return true;
    }
}

static void extractCsiDataContent() {

}

static void runFacialRecognition() {

}

static bool compareSignalLocToFaceLoc() {
    // Compute the angles of either side of the face detected
    // Include tolerance of 2-5 degrees on either end
    // This tolerance is what I should be measuring for some of my experiments
}

//~Tests and whatnot--------------------------------------------------------------------------------
/**
 * Run collectCsiData while running a ping command too.
 */
static void testCollectCsiData() {
    
}