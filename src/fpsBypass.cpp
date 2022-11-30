#include "fpsBypass.h"

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
        if (fpsbypass_enabled) {
            void *application = sharedApplication();
            setAnimInterval(application, 1.0f / g_target_fps);
        }   
        if (gd::PlayLayer::get() && !gd::PlayLayer::get()->m_bIsPaused) {      
            if (!g_enabled)
                return CCScheduler_update(self, dt);
            auto speedhack = self->getTimeScale();
            const float newdt = 1.f / g_target_fps / speedhack;
            g_disable_render = true;

            const int times = min(static_cast<int>((dt + g_left_over) / newdt), 100);
            for (int i = 0; i < times; ++i) {
                if (i == times - 1)
                    g_disable_render = false;
                if (frame_advance && !nextframe) {return;}
                else {nextframe = false;}
                CCScheduler_update(self, newdt);
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
