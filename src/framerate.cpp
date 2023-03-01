#include "framerate.h"
#include "replayEngine.h"

namespace framerate{
    bool g_disable_render = true;
    float g_left_over = 0.f;

    void (__thiscall* CCScheduler_update)(CCScheduler*, float);
    void __fastcall CCScheduler_update_H(CCScheduler* self, int, float dt) {
        auto pl = gd::GameManager::sharedState()->getPlayLayer();
        auto ey = gd::GameManager::sharedState()->getEditorLayer();
        dt *= dashreplay::info::speedhack;
        if (ey || pl && !pl->m_isPaused) {                 
            const float newdt = 1.f / dashreplay::info::fps / self->getTimeScale();
            unsigned times = static_cast<int>((dt + g_left_over) / newdt);
            if (dt == 0.f) {
                if (!dashreplay::frameadvance::enabled)
                    return CCScheduler_update(self, newdt);
            }
                
            auto start = std::chrono::high_resolution_clock::now();

            if (dashreplay::frameadvance::enabled) {
                if (!dashreplay::frameadvance::next_frame)
                    return;

                CCScheduler_update(self, newdt);
                dashreplay::frameadvance::next_frame = false;
                return;
            }

            for (unsigned i = 0; i < times; ++i) {
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

    void mem_init() {
        auto base = GetModuleHandle(0);
        auto cocos = GetModuleHandleA("libcocos2d.dll");
        MH_CreateHook(GetProcAddress(cocos, "?update@CCScheduler@cocos2d@@UAEXM@Z"), CCScheduler_update_H, (void**)&CCScheduler_update);
    }

}
