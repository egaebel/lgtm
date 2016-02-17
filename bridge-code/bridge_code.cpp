#define DEBUG 1

// C++ headers
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

// Linux headers
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// My Headers

//~Constants----------------------------------------------------------------------------------------
static const std::string matlabOutputFile = "matlab_aoa_output.out";
/*
static const std::string matlabCommand = (std::string) "matlab -nojvm -nodisplay -nosplash -r "
        + "\"run('/home/egaebel/grad-docs/research/thesis/csi-code/tests/spotfi_test.m'), exit\""
        + " > " 
        + matlabOutputFile;
*/
static const std::string csiDataFile = "csi.dat";
static const std::string csiCollectionOutputFile = "csi-log-collection.out";
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
    if (DEBUG) {
        std::cout << std::endl << std::endl << std::endl;
    }
    // Analyze signal from other user with MATLAB and get angle of arrival
    std::vector<double> angleOfArrivals;
    runMatlab(angleOfArrivals);
    // Extract data sent from other user and build facial recognition model
    // extractCsiDataContent();
    // Run facial recognition model on current video stream
    // runFacialRecognition();s
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
    if (DEBUG) {
        std::cout << "DEBUG: Running collectCsiData!" << std::endl;
    }
    using namespace std::chrono;
    // 5 minute timeout
    const int TIMEOUT = 60 * 5;
    // Run command, parse output file to determine when to kill command
    pid_t forkRetVal = fork();
    // Error
    if (forkRetVal == -1) {
        std::cout << "ERROR: on fork in collectCsiData: " << strerror(errno) << std::endl;
    // Child
    } else if (forkRetVal == 0) {
        if (DEBUG) {
            std::cout << "DEBUG: Running CSI collection in child!" << std::endl;
        }
        int fd = open(csiCollectionOutputFile.c_str(), 
                O_RDWR | O_TRUNC | O_CREAT, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0) {
            std::cout << "ERROR: on opening " << csiCollectionOutputFile << " in collectCsiData: " 
                    << strerror(errno) << std::endl;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            std::cout << "ERROR: on dup2 sending stdout to fd in collectCsiData: " 
                    << strerror(errno) << std::endl;
        }
        close(fd);
        char* const args[] = {(char*) "unbuffer", (char*) "./csi-collection.sh", (char*) 0};
        int retVal = execvp(args[0], args);
        if (retVal < 0) {
            std::cout << "ERROR: on execvp executing: " << args[0] << " in collectCsiData: "
                    << strerror(errno) << std::endl;
        }
        if (DEBUG) {
            std::cout << "DEBUG: Finished running CSI collection in child!" << std::endl;
        }
    // Parent
    } else {
        if (DEBUG) {
            std::cout << "DEBUG: Parsing file in parent!" << std::endl;
        }
        int linesWritten = 0;
        steady_clock::time_point beginTime = steady_clock::now();
        steady_clock::time_point endTime = steady_clock::now();
        duration<int> timeElapsed = duration_cast<duration<int>>(endTime - beginTime);
        if (DEBUG) {
            std::cout << "DEBUG: timeElapsed.count() == " << timeElapsed.count() << std::endl;
            std::cout << "DEBUG: TIMEOUT == " << TIMEOUT << std::endl;
            std::cout << "DEBUG: linesWritten == " << linesWritten << std::endl;
            std::cout << "DEBUG: MAX_NUM_CSI_OUTPUT_LINES == " << MAX_NUM_CSI_OUTPUT_LINES << std::endl;
        }
        while (linesWritten < MAX_NUM_CSI_OUTPUT_LINES && timeElapsed.count() < TIMEOUT) {
            // Parse output file
            linesWritten = 0;
            std::ifstream infile(csiCollectionOutputFile);
            std::string line;
            while (std::getline(infile, line)) {
                linesWritten += 1;
            }
            infile.close();
            timeElapsed = duration_cast<duration<int>>(endTime - beginTime);
            if (DEBUG) {
                std::cout << "DEBUG: Parsed " << linesWritten << " lines in " 
                        << csiCollectionOutputFile << std::endl;
            }
        }
        if (timeElapsed.count() >= TIMEOUT) {
            std::cout << "ERROR: Timeout was reached when parsing " 
                    << csiCollectionOutputFile << std::endl;
        }
        if (DEBUG) {
            std::cout << "DEBUG: Killing csi collection!" << std::endl;
        }
        int killRetVal = kill(forkRetVal, SIGINT);
        if (killRetVal < 0) {
            std::cout << "ERROR: on killing child: " << strerror(errno) << std::endl;
        }
    }
}

static bool runMatlab(std::vector<double> &angleOfArrivals) {
    if (DEBUG) {
        std::cout << "DEBUG: Running runMatlab" << std::endl;
    }
    pid_t forkRetVal = fork();
    if (forkRetVal == -1) {
        std::cout << "ERROR: on fork in runMatlab: " << strerror(errno) << std::endl;
        return false;
    } else if (forkRetVal == 0) {
        if (DEBUG) {
            std::cout << "DEBUG: Running matlab in child!" << std::endl;
        }
        int fd = open(matlabOutputFile.c_str(), 
                O_RDWR | O_TRUNC | O_CREAT, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0) {
            std::cout << "ERROR: on opening " << matlabOutputFile << " in runMatlab: " 
                    << strerror(errno) << std::endl;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            std::cout << "ERROR: on dup2 sending stdout to fd in runMatlab: " << strerror(errno) << std::endl;
        }
        close(fd);
        // NOTE:
        // The below is very stupid and not portable at all, however it is necessary since I must
        // run this program as root to execute the csi-collection.sh script and MATLAB's license 
        // only binds to a SINGLE USER on the machine...so running MATLAB as root prompts me to
        // enter in license stuff, which is rejected anyway since MATLAB is already bound to my
        // user, egaebel.
        // TODO:
        // The best solution I think is to have a config file where the user has to enter in their
        // username, then that config file is parsed and the username is put in here in the 
        // matlabUsername field. This solution is REALLY, REALLY STUPID, 
        // and it's all MathWorks' fault
        char* matlabUsername = (char*) "egaebel";
        char* const args[] = {(char*) "sudo", (char*) "-u", matlabUsername, 
                (char*) "matlab", (char*) "-nojvm", (char*) "-nodisplay", 
                (char*) "-nosplash", 
                (char*) "-r", 
                (char*) "try, run('../csi-code/test-code/spotfi_test.m'), catch, end, exit", 
                (char*) 0};
        int retVal = execvp(args[0], args);
        if (retVal < 0) {
            std::cout << "ERROR: on execvp executing: " << args[0] << " in runMatlab: "
                    << strerror(errno) << std::endl;
        }
        return false;
    } else {
        if (DEBUG) {
            std::cout << "DEBUG: Waiting for matlab in parent!" << std::endl;
        }
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
            if (DEBUG) {
                std::cout << "DEBUG: Read " << line << " from " << matlabOutputFile << std::endl;
            }
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