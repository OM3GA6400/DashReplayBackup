#include "hooks.h"
#include "replayEngine.h"

namespace hooks {
    void __fastcall dispatchKeyboardMSGHook(void* self, void*, int key, bool down) {
        dispatchKeyboardMSG(self, key, down);
        auto pl = gd::GameManager::sharedState()->getPlayLayer();
        if (pl && down && key == 'C') {
            dashreplay::frameadvance::enabled = true;
            dashreplay::frameadvance::next_frame = true;          
        }
        else if (pl && down && key == 'F') {
            dashreplay::frameadvance::enabled = false;
            dashreplay::frameadvance::next_frame = false;  
        }
        else if (pl && down && key == 'P') {
            if (dashreplay::info::mode != state::play) dashreplay::info::mode = state::play;
            else dashreplay::info::mode = state::disable;
        }
    }

    void mem_init() {
        MH_CreateHook((PVOID)(GetProcAddress(GetModuleHandleA("libcocos2d.dll"),
            "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z")),
                dispatchKeyboardMSGHook, (LPVOID*)&dispatchKeyboardMSG);
    }
}
