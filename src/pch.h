#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cocos2d.h>
#include <MinHook.h>
#include <gd.h>
#include <vector>
#include <fstream>
#include <shellapi.h>
#include "json.hpp"
#include "console.hpp"
#include "check.h"
#include "memory.h"

using namespace std;
using namespace cocos2d;

using json = nlohmann::json;

#endif