#include "hooks.h"
#include "replayEngine.h"
#include "spambot.h"
#include "imgui-hook.hpp"
#include "config.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hooks {
    void (__thiscall* CCEGLView_pollEvents)(CCEGLView*);
    void __fastcall CCEGLView_pollEvents_H(CCEGLView* self) {
        auto& io = ImGui::GetIO();

        bool blockInput = false;
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);

            if (io.WantCaptureMouse) {
                switch(msg.message) {
                    case WM_LBUTTONDBLCLK:
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                    case WM_MBUTTONDBLCLK:
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                    case WM_MOUSEACTIVATE:
                    case WM_MOUSEHOVER:
                    case WM_MOUSEHWHEEL:
                    case WM_MOUSELEAVE:
                    case WM_MOUSEMOVE:
                    case WM_MOUSEWHEEL:
                    case WM_NCLBUTTONDBLCLK:
                    case WM_NCLBUTTONDOWN:
                    case WM_NCLBUTTONUP:
                    case WM_NCMBUTTONDBLCLK:
                    case WM_NCMBUTTONDOWN:
                    case WM_NCMBUTTONUP:
                    case WM_NCMOUSEHOVER:
                    case WM_NCMOUSELEAVE:
                    case WM_NCMOUSEMOVE:
                    case WM_NCRBUTTONDBLCLK:
                    case WM_NCRBUTTONDOWN:
                    case WM_NCRBUTTONUP:
                    case WM_NCXBUTTONDBLCLK:
                    case WM_NCXBUTTONDOWN:
                    case WM_NCXBUTTONUP:
                    case WM_RBUTTONDBLCLK:
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                    case WM_XBUTTONDBLCLK:
                    case WM_XBUTTONDOWN:
                    case WM_XBUTTONUP:
                        blockInput = true;
                }
            }

            if (io.WantCaptureKeyboard) {
                switch(msg.message) {
                    case WM_HOTKEY:
                    case WM_KEYDOWN:
                    case WM_KEYUP:
                    case WM_KILLFOCUS:
                    case WM_SETFOCUS:
                    case WM_SYSKEYDOWN:
                    case WM_SYSKEYUP:
                        blockInput = true;
                }
            } else if (msg.message == WM_KEYDOWN && msg.wParam == config::keybind_menu) {
               ImGuiHook::g_toggleCallback();
            } else if (msg.message == WM_KEYDOWN && msg.wParam == config::keybind_frameadvance) {
                dashreplay::frameadvance::enabled = !dashreplay::frameadvance::enabled;
            } else if (msg.message == WM_KEYDOWN && msg.wParam == config::keybind_nextframe) {
                dashreplay::frameadvance::next_frame = true;
            } else if (msg.message == WM_KEYDOWN && msg.wParam == config::keybind_spambot) {
                spambot::enable = !spambot::enable;
            } else if (msg.message == WM_KEYDOWN && msg.wParam == 'P') {
                if (dashreplay::info::mode != state::play) dashreplay::info::mode = state::play;
                else dashreplay::info::mode = state::disable;
            }

            if (!blockInput)
                DispatchMessage(&msg);

            ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);
        }

        CCEGLView_pollEvents(self);
    }

    void mem_init() {
        MH_CreateHook(GetProcAddress(GetModuleHandleA("libcocos2d.dll"), "?pollEvents@CCEGLView@cocos2d@@QAEXXZ"), CCEGLView_pollEvents_H,
            reinterpret_cast<void**>(&CCEGLView_pollEvents));
    }
}
