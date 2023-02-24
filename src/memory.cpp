#include "memory.h"

namespace memory {
    HWND window = NULL;

    void get_hwnd() {
        DWORD processId = NULL;
        while (processId != GetCurrentProcessId()) {
            window = GetForegroundWindow();
            if (window != GetConsoleWindow())
                GetWindowThreadProcessId(window, &processId);
        }
    }
}