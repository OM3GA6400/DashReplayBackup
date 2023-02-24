#include "pch.h"

namespace hooks {
    inline void(__thiscall* dispatchKeyboardMSG)(void* self, int key, bool down);
    void __fastcall dispatchKeyboardMSGHook(void* self, void*, int key, bool down);

    void mem_init();
}

