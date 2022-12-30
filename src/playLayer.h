#include <windows.h>
#include <cocos2d.h>
#include <gd.h>
#include <MinHook.h>
#include <fstream>
#include <vector>
#include "fpsBypass.h"
#include "uselessShit.h"
#include "spamBot.h"
#include "console.h"
#include <thread>

using namespace std;

struct replaydata {
	int frame;
	float pos_x;
	float pos_y;
	float rotation;
	float y_vel;
	int down;
};

struct checkpointsdata {
	int frame;
    float pos_x;
    float pos_y;
    float rotation;
    float x_vel;
    float y_vel;
    float jump_vel;
    float player_speed; //robtop i love your work
    bool is_upsidedown;
};

namespace playLayer {
    extern int frame;
    
    extern int framestart;
    extern int mode;

    extern bool down_p1;
    extern bool down_p2;

    extern bool practice_fix;
    extern bool accuracy_fix;
    extern bool rotation_fix;

    extern bool ignore_input;
    extern bool dual_clicks;

    extern float fpsvalue;
    extern float speedvalue;

    extern bool enable_sqp;
    extern bool random_sqp;
    extern int current_index_sqp;

    extern vector<string> macro_sqp;
    extern int sqp_current_idx;
    extern bool first_sqp;
    extern bool loadMacroASync;

    bool loadReplay(string s);
    bool saveReplay(string s);
    void clearMacro();

    extern vector<replaydata> replay_p1;
    extern vector<replaydata> replay_p2;

    extern vector<checkpointsdata> checkpoints_p1;
    extern vector<checkpointsdata> checkpoints_p2;

    inline bool(__thiscall* init)(gd::PlayLayer* self, gd::GJGameLevel* GJGameLevel);
    bool __fastcall initHook(gd::PlayLayer* self, int edx, gd::GJGameLevel* GJGameLevel);

    inline void(__thiscall* update)(gd::PlayLayer* self, float deltatime);
    void __fastcall updateHook(gd::PlayLayer* self, int edx, float deltatime);

    inline void(__thiscall* resetLevel)(gd::PlayLayer* self);
    void __fastcall resetLevelHook(gd::PlayLayer* self);

    inline void(__thiscall* onQuit)(gd::PlayLayer* self);
    void __fastcall onQuitHook(gd::PlayLayer* self);
        
    inline void(__thiscall* levelComplete)(gd::PlayLayer* self);
    void __fastcall levelCompleteHook(gd::PlayLayer* self);

    inline int(__thiscall* death)(void* self, void* go, void* powerrangers);
	int __fastcall deathHook(gd::PlayLayer* self, void*, void* go, void* powerrangers);

    inline bool(__thiscall* pushButton)(gd::PlayLayer* self, int state, bool player);
	bool __fastcall pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player);

	inline bool(__thiscall* releaseButton)(gd::PlayLayer* self, int state, bool player);
	bool __fastcall releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player);

    inline int(__thiscall* createCheckpoint)(gd::PlayLayer* self);
    int __fastcall createCheckpointHook(gd::PlayLayer* self);

    inline int(__thiscall* removeCheckpoint)(gd::PlayLayer* self);
    int __fastcall removeCheckpointHook(gd::PlayLayer* self);

    void mem_init();
}

