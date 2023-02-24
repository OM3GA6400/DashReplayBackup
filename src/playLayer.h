#include "pch.h"

namespace playLayer {
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

    inline int(__thiscall* createCheckpoint)(gd::PlayLayer* self);
    int __fastcall createCheckpointHook(gd::PlayLayer* self);   

    inline int(__thiscall* removeCheckpoint)(gd::PlayLayer* self);
    int __fastcall removeCheckpointHook(gd::PlayLayer* self);   

    inline bool(__thiscall* pushButton)(gd::PlayLayer* self, int state, bool player);
    bool __fastcall pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player); 

    inline bool(__thiscall* releaseButton)(gd::PlayLayer* self, int state, bool player);
    bool __fastcall releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player);

    inline void(__thiscall* togglePractice)(void* self, bool practice);
    void __fastcall togglePracticeHook(void* self, int edx, bool practice); 

    inline bool(__thiscall* onEditorButtonClick)(gd::LevelEditorLayer* self, gd::GJGameLevel* GJGameLevel);
    bool __fastcall onEditorButtonClickHook(gd::LevelEditorLayer* self, int edx, gd::GJGameLevel* GJGameLevel); 

    void mem_init();
}