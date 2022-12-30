#define _CRT_SECURE_NO_WARNINGS
#include "fpsBypass.h"
#include "playLayer.h"

using namespace cocos2d;

typedef void*   (__cdecl *fSharedApplication)();
typedef void    (__thiscall *fSetAnimationInterval)(void *instance, double delay);
fSharedApplication sharedApplication;
fSetAnimationInterval setAnimInterval;

namespace FPSMultiplier{
    float FPSMultiplier::g_target_fps = 240.f;
    bool FPSMultiplier::g_enabled = true;
    bool FPSMultiplier::fpsbypass_enabled = true;
    bool FPSMultiplier::g_disable_render = true;
    float FPSMultiplier::g_left_over = 0.f;

    bool FPSMultiplier::nextframe = false;
    bool FPSMultiplier::frame_advance = false;

    void (__thiscall* CCScheduler_update)(CCScheduler*, float);
    void __fastcall CCScheduler_update_H(CCScheduler* self, int, float dt) {
        auto pl = gd::GameManager::sharedState()->getPlayLayer();
        if (fpsbypass_enabled) {
            void *application = sharedApplication();
            setAnimInterval(application, 1.0f / g_target_fps);
        }   
        if (pl && !pl->m_isPaused) {      
            if (!g_enabled)
                return CCScheduler_update(self, dt);
            auto speedhack = self->getTimeScale();
            const float newdt = 1.f / g_target_fps / speedhack;
            g_disable_render = true;

            unsigned times = static_cast<int>((dt + g_left_over) / newdt);
            if (dt == 0.f)
                return CCScheduler_update(self, newdt);
            auto start = std::chrono::high_resolution_clock::now();
            for (unsigned i = 0; i < times; ++i) {
                if (frame_advance && !nextframe) {return;}
                else {nextframe = false;}
                CCScheduler_update(self, newdt);
                using namespace std::literals;
                if (std::chrono::high_resolution_clock::now() - start > 33.333ms) {
                    times = i + 1;
                    break;
                }
            }
            g_left_over += dt - newdt * times;
        } else {
            CCScheduler_update(self, dt);
        }
    }

    void (__thiscall* PlayLayer_updateVisibility)(void*);
    void __fastcall PlayLayer_updateVisibility_H(void* self) {
        if (!g_disable_render)
            PlayLayer_updateVisibility(self);
    }

    void FPSMultiplier::Setup() {
        HMODULE hMod = LoadLibrary("libcocos2d.dll");
        sharedApplication = (fSharedApplication)GetProcAddress(hMod, "?sharedApplication@CCApplication@cocos2d@@SAPAV12@XZ");
        setAnimInterval = (fSetAnimationInterval)GetProcAddress(hMod, "?setAnimationInterval@CCApplication@cocos2d@@UAEXN@Z");
        MH_Initialize();
        auto base = GetModuleHandle(0);
        auto cocos = GetModuleHandleA("libcocos2d.dll");
        MH_CreateHook((void*)(base + 0x205460), PlayLayer_updateVisibility_H, (void**)&PlayLayer_updateVisibility);
        MH_CreateHook(GetProcAddress(cocos, "?update@CCScheduler@cocos2d@@UAEXM@Z"), CCScheduler_update_H, (void**)&CCScheduler_update);
    }

}
