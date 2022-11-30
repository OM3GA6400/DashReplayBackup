#include <windows.h>
#include <cocos2d.h>
#include <gd.h>
#include <MinHook.h>

using namespace std;

namespace FPSMultiplier {
    extern bool nextframe;
    extern bool frame_advance;
    extern float g_target_fps;
    extern bool g_enabled;
    extern bool fpsbypass_enabled;
    extern bool g_disable_render;
    extern float g_left_over;
    void Setup();
}

