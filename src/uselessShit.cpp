#include "uselessShit.h"

ImFont* uselessShit::fontBigger;

namespace uselessShit {
    bool uselessShit::stringToBool(string s) {
        if (s == "true") 
            return true;
        else
            return false;
    }

    string uselessShit::boolToString(bool b) {
        if (b) 
            return "true";
        else
            return "false";
    }

    int uselessShit::random_range(int from, int to) {
        float random = ((float)rand()) / (float)RAND_MAX;
        float diff = to - from;
        float r = random * diff;
        return (int)(from + r);
    }
}