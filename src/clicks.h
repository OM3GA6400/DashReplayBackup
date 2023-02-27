#include "pch.h"
#include <imgui.h>

using namespace std::filesystem;

namespace clicks {
    extern vector<string> clickpacks;
    extern char output[128];
    extern bool softclicks;
    extern int softdelay;
    extern int selected_clickpack_idx;

    extern bool hardclicks;
    extern int harddelay;

    extern bool include_clicks;
    bool do_clicks(bool msg);
    void render();
    void update_list();
}

const string structure = "[clickpack name]\n\
| \n\
+ softclicks (optional)\n\
| |\n\
| + 1.wav\n\
| + 2.wav\n\
| + 3.wav\n\
+ hardclicks (optional)\n\
| |\n\
| + 1.wav\n\
| + 2.wav\n\
| + 3.wav\n\
+ p1\n\
| |\n\
| + holds\n\
| | |\n\
| | + 1.wav\n\
| | + 2.wav\n\
| | + 3.wav\n\
| + releases\n\
|   |\n\
|   + 1.wav\n\
|   + 2.wav\n\
|   + 3.wav\n\
+ p2 (optional)\n\
  |\n\
  + holds\n\
  | |\n\
  | + 1.wav\n\
  | + 2.wav\n\
  | + 3.wav\n\
  + releases\n\
    |\n\
    + 1.wav\n\
    + 2.wav\n\
    + 3.wav";