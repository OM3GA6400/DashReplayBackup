#include "config.h"
#include "replayEngine.h"
#include "json.hpp"

namespace config {
    int keybind_menu = 'K';
    int keybind_frameadvance = 'F';
    int keybind_nextframe = 'C';
    int keybind_spambot = 'S';
    int keybind_playback = 'P';

    void save() {
        json data; 
        data["fps"] = dashreplay::info::fps;
        data["speedhack"] = dashreplay::info::speedhack;
        data["keybind_menu"] = keybind_menu;
        data["keybind_frameadvance"] = keybind_frameadvance;
        data["keybind_nextframe"] = keybind_nextframe;
        data["keybind_spambot"] = keybind_spambot;
        data["keybind_playback"] = keybind_playback;
        ofstream file("DashReplay\\Config.json");
        file << std::setw(4) << data << std::endl;
    }

    void load() {
        ifstream file("DashReplay\\Config.json");
        string file_content;
        getline(file, file_content, '\0');
        file.close();

        dashreplay::info::fps = json::parse(file_content)["fps"];
        dashreplay::info::speedhack = json::parse(file_content)["speedhack"];
        keybind_menu = json::parse(file_content)["keybind_menu"];
        keybind_frameadvance = json::parse(file_content)["keybind_frameadvance"];
        keybind_nextframe = json::parse(file_content)["keybind_nextframe"];
        keybind_spambot = json::parse(file_content)["keybind_spambot"];
        keybind_playback = json::parse(file_content)["keybind_playback"];
    }
}