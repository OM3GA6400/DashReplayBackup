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
}