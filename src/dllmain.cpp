#include <windows.h>
#include <shellapi.h>
#include <cocos2d.h>
#include <gd.h>
#include <string.h>
#include "console.h"
#include "fpsBypass.h"
#include "playLayer.h"
#include "spamBot.h"
#include <imgui_hook.h>
#include <imgui_internal.h>
#include <imgui.h>

using namespace cocos2d;

bool nolcip;
bool practice_music_hack;
bool practice_coins;
bool anticheat_bypass;
bool ignore_esc;
bool no_respawn_flash;
bool disable_death_effects;

bool show = false;
bool inited = false;

bool isRecording;

const char* converterTypes[]{"Plain Text (.txt)"};
int converterType = 0;

vector<string> items = { "General", "Assist", "Editor", "Converter", "Sequential Play", "Hacks", "About"};
int item_current_idx = 0;

int replay_select_player_p1 = 1;
int replay_current = 0;

char replay_name[128] = "";
vector<string> replay_list;
bool openned = false;

bool overwrite = false;
bool loading = false;

void ConfirmMessage(float x, float y) {
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(280, 75));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    ImGui::Begin("##nani", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("Warning!");
    ImGui::Text("This will overwrite your currently loaded replay");
    if (ImGui::Button("Yes")) {
        if (loading) {
            playLayer::clearMacro();
            playLayer::loadReplay(".DashReplay/" + (string)replay_name);
        }
        else {
            playLayer::clearMacro();
        }
        overwrite = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("No")) {
        overwrite = false;
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void RenderMain() {
    CCDirector::sharedDirector()->getTouchDispatcher()->setDispatchEvents(!ImGui::GetIO().WantCaptureMouse);
    if (show) {
        ImGui::Begin("DashReplay", &show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
        if (!inited) {
            ImGui::SetWindowPos(ImVec2(10, 10));
            ImGui::SetWindowSize(ImVec2(500, 300));
            inited = true;
        }

        if (ImGui::BeginChild("##LeftSide", ImVec2(120, ImGui::GetContentRegionAvail().y), true))
        {	
            for (int i = 0; i < (int)items.size(); i++)
            {
                const bool is_selected = (item_current_idx == i);
                if (ImGui::Selectable(items[i].c_str(), is_selected))
                    item_current_idx = i;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndChild();
        }

        ImGui::SameLine();

        if (ImGui::BeginChild("##RigthSide", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true))
        {
            if (item_current_idx == 0) {
                if (ImGui::RadioButton("Disable", &playLayer::mode, 0)) {
                    playLayer::checkpoints_p1.clear();
                    playLayer::checkpoints_p2.clear();
                }

                ImGui::SameLine();

                if (ImGui::RadioButton("Record", &playLayer::mode, 1)) {
                     if (practice_music_hack && anticheat_bypass) {
                        playLayer::replay_p1.clear();
                        playLayer::replay_p2.clear();
                        playLayer::checkpoints_p1.clear();
                        playLayer::checkpoints_p2.clear();
                    }
                    else {
                        playLayer::mode = 0;
                    }
                }

                if (!practice_music_hack || !anticheat_bypass) {
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Enable \"Practice Music Hack\" and \"Anticheat bypass\"");
                }

                ImGui::SameLine();

                ImGui::RadioButton("Play", &playLayer::mode, 2);

                ImGui::Separator();

                ImGui::InputText("##replayinput", replay_name, IM_ARRAYSIZE(replay_name));
                auto itemx = ImGui::GetItemRectMin().x;
                auto itemy = ImGui::GetItemRectMax().y;
                auto itemw = ImGui::GetItemRectSize().x;
                ImGui::SameLine(0);
                if (ImGui::ArrowButton("##comboopen", openned ? ImGuiDir_Up : ImGuiDir_Down))  {
                    openned = !openned; 
                    if (openned) {
                        replay_list.clear();
                        for (const auto & entry : filesystem::directory_iterator(".DashReplay")) {
                            replay_list.push_back(entry.path().filename().string());
                        }
                    }
                }   
                if (openned) {
                    ImGui::SetNextWindowPos(ImVec2(itemx, itemy + 4));
                    ImGui::SetNextWindowSize(ImVec2(itemw + ImGui::GetItemRectSize().x, NULL));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
                    ImGui::Begin("##MacroList", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                    for (int i = 0; i < (int)replay_list.size(); i++) {
                        if (ImGui::MenuItem(replay_list[i].c_str())) {
                            strcpy_s(replay_name, replay_list[i].c_str());
                            openned = false;
                        }
                    }            
                    ImGui::End();
                    ImGui::PopStyleVar();
                }

                if (ImGui::Button("Load", {60, NULL})) { 
                    if (playLayer::replay_p1.empty()) {
                        playLayer::clearMacro();
                        playLayer::loadReplay(".DashReplay/" + (string)replay_name);
                    }
                    else {
                        overwrite = true;
                        loading = true;
                    }                   

                }

                if (overwrite) {
                    ConfirmMessage(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y + 4);
                }

                ImGui::SameLine();

                if (ImGui::Button("Save", {60, NULL})) {           
                    playLayer::saveReplay(".DashReplay/" + (string)replay_name);
                }

                ImGui::SameLine();
                if (ImGui::Button("Clear Replay", {80, NULL})) {
                    if (playLayer::replay_p1.empty()) playLayer::clearMacro();
                    else  {
                        overwrite = true;
                        loading = false;
                    }
                }
                ImGui::Separator();

                ImGui::PushItemWidth(160.f);
                ImGui::DragFloat("##FPS", &FPSMultiplier::g_target_fps, 1.f, 1.f, FLT_MAX, "FPS: %.2f");

                ImGui::SameLine();

                ImGui::PushItemWidth(160.f);
                if (ImGui::DragFloat("##Speed", &playLayer::speedvalue, 0.01f, 0.f, FLT_MAX, "Speed: %.2f")) {
                    if (playLayer::speedvalue != 0) { CCDirector::sharedDirector()->getScheduler()->setTimeScale(playLayer::speedvalue); }
                }

                ImGui::Separator();
                ImGui::Checkbox("Practice Fix", &playLayer::practice_fix);
                ImGui::SameLine();
                ImGui::Checkbox("Accuracy Fix", &playLayer::accuracy_fix);
                if (playLayer::accuracy_fix) {ImGui::SameLine(); ImGui::Checkbox("Rotation Fix", &playLayer::rotation_fix);}
                ImGui::Separator();
                ImGui::Checkbox("FPS Bypass", &FPSMultiplier::fpsbypass_enabled);
                ImGui::SameLine();
                ImGui::Checkbox("FPS Multiplier", &FPSMultiplier::g_enabled);
                ImGui::Separator();
                ImGui::Checkbox("Ignore Inputs on Playback", &playLayer::ignore_input);
                ImGui::Separator();
                ImGui::Text("Frame: %i", playLayer::frame);
                ImGui::Text("Replay Size: %i (P2: %i)", (int)playLayer::replay_p1.size(), (int)playLayer::replay_p2.size());
            }

            if (item_current_idx == 1) {
                ImGui::Checkbox("Frame Advance", &FPSMultiplier::frame_advance);
                ImGui::Separator();
                ImGui::Checkbox("Spam Bot", &spambot::enable);

                ImGui::SameLine();
                ImGui::PushItemWidth(100.f);	
                ImGui::DragInt("##spampush", &spambot::push, 1, 1, INT_MAX, "Push: %i");

                ImGui::SameLine();
                ImGui::PushItemWidth(100.f);	
                ImGui::DragInt("##spamreelase", &spambot::release, 1, 1, INT_MAX, "Release: %i");

                ImGui::Checkbox("1 Player", &spambot::player1);
                ImGui::SameLine();
                ImGui::Checkbox("2 Player", &spambot::player2);
                ImGui::Separator();  
                ImGui::Checkbox("Dual Clicks", &playLayer::dual_clicks);          

            }

            if (item_current_idx == 2) {
                if (ImGui::BeginChild("##LeftSideEditor", ImVec2(120, ImGui::GetContentRegionAvail().y), true))
                {	
                    for (int i = 0; i < (int)playLayer::replay_p1.size(); i++)
                    {
                        const bool is_selected = (replay_current == i);
                        ImGui::PushItemWidth(120.f);
                        if (ImGui::Selectable(to_string(playLayer::replay_p1[i].frame).c_str(), is_selected))
                            replay_current = i;

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndChild();
                }


                ImGui::SameLine();

                if (ImGui::BeginChild("##RightSideEditor", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true))
                {	
                    if (!playLayer::replay_p1.empty() && replay_select_player_p1) {
                        ImGui::DragFloat("##POSXP1", &playLayer::replay_p1[replay_current].pos_x, 0.000001f, -1, FLT_MAX, "Position X: %f");
                        ImGui::DragFloat("##POSYP1", &playLayer::replay_p1[replay_current].pos_y, 0.000001f, -1, FLT_MAX, "Position Y: %f");
                        ImGui::DragFloat("##ROTATEP1", &playLayer::replay_p1[replay_current].rotation, 0.000001f, -1, FLT_MAX, "Rotation: %f");
                        ImGui::DragFloat("##YVELP1", &playLayer::replay_p1[replay_current].y_vel, 0.000001f, -1, FLT_MAX, "Y Vel: %f");
                        ImGui::DragInt("##DOWNP1", &playLayer::replay_p1[replay_current].down, 1, -1, 1, "Down: %i");
                    }

                    if (!playLayer::replay_p2.empty() && !replay_select_player_p1) {
                        ImGui::DragFloat("##POSXP2", &playLayer::replay_p2[replay_current].pos_x, 0.000001f, -1, FLT_MAX, "Position X: %f");
                        ImGui::DragFloat("##POSYP2", &playLayer::replay_p2[replay_current].pos_y, 0.000001f, -1, FLT_MAX, "Position Y: %f");
                        ImGui::DragFloat("##ROTATEP2", &playLayer::replay_p2[replay_current].rotation, 0.000001f, -1, FLT_MAX, "Rotation: %f");
                        ImGui::DragFloat("##YVELP2", &playLayer::replay_p2[replay_current].y_vel, 0.000001f, -1, FLT_MAX, "Y Vel: %f");
                        ImGui::DragInt("##DOWNP2", &playLayer::replay_p2[replay_current].down, 1, -1, 1, "Down: %i");
                    }    
                    ImGui::Text("Note: -1 value does nothing with\nplayer");
                    ImGui::Separator();
                    ImGui::RadioButton("Player 1", &replay_select_player_p1, 1);
                    ImGui::SameLine();
                    ImGui::RadioButton("Player 2", &replay_select_player_p1, 0);
                    ImGui::Separator();
                    ImGui::EndChild();
                }
            }

            if (item_current_idx == 3) {
                ImGui::Combo("##ConverterType", &converterType, converterTypes, IM_ARRAYSIZE(converterTypes));
                if (ImGui::Button("Convert")) {
                    if (converterType == 0) {
                        if (converterType == 0) {
                            std::ofstream out(".DashReplay/converted.txt");
                            out << FPSMultiplier::g_target_fps << "\n";
                            for (int i = 0; i < (int)playLayer::replay_p1.size(); i++) {
                                if (i == 0 || (playLayer::replay_p1[i].down == playLayer::replay_p1[i - 1].down && playLayer::replay_p2[i].down == playLayer::replay_p2[i - 1].down))
                                    continue;
                                out << i << " " << playLayer::replay_p1[i].down << " " << playLayer::replay_p2[i].down << "\n";
                            }
                            out.close();	
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Matcool Converter")) {
                    ShellExecuteA(0, "open", "https://matcool.github.io/gd-macro-converter/", 0, 0, SW_SHOWNORMAL);
                }
                if (converterType == 0) {
                    ImGui::Text("Replay will be saved to \".DashReplay/converted.txt\"");
                }
            }

            if (item_current_idx == 4) {
                ImGui::Checkbox("Toggle Sequential Play", &playLayer::enable_sqp);
                ImGui::SameLine();
                ImGui::Checkbox("Random Replay", &playLayer::random_sqp);
                ImGui::InputText("##replayinputinsqp", replay_name, IM_ARRAYSIZE(replay_name));
                auto itemx = ImGui::GetItemRectMin().x;
                auto itemy = ImGui::GetItemRectMax().y;
                auto itemw = ImGui::GetItemRectSize().x;
                ImGui::SameLine();
                if (ImGui::ArrowButton("##comboopeninsqp", openned ? ImGuiDir_Up : ImGuiDir_Down))  {
                    openned = !openned; 
                    if (openned) {
                        replay_list.clear();
                        for (const auto & entry : filesystem::directory_iterator(".DashReplay")) {
                            replay_list.push_back(entry.path().filename().string());
                        }
                    }
                }   
                if (openned) {
                    ImGui::SetNextWindowPos(ImVec2(itemx, itemy + 4));
                    ImGui::SetNextWindowSize(ImVec2(itemw + ImGui::GetItemRectSize().x, NULL));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
                    ImGui::Begin("##MacroListInSQP", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
                    for (int i = 0; i < (int)replay_list.size(); i++) {
                        if (ImGui::MenuItem(replay_list[i].c_str())) {
                            strcpy_s(replay_name, replay_list[i].c_str());
                            openned = false;
                        }
                    }            
                    ImGui::End();
                    ImGui::PopStyleVar();
                }

                ImGui::SameLine();

                if (ImGui::Button("Add")) {
                    playLayer::macro_sqp.push_back((string)replay_name);
                }

                ImGui::Text("Current Replay %i/%i", playLayer::sqp_current_idx + 1, playLayer::macro_sqp.size());
                
                if (ImGui::BeginChild("##SQPPanel", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true))
                {	
                    for (int i = 0; i < (int)playLayer::macro_sqp.size(); i++)
                    {
                        if (ImGui::MenuItem(playLayer::macro_sqp[i].c_str())) {
                            playLayer::macro_sqp.erase(playLayer::macro_sqp.begin()+i);
                        }
                    }
                    ImGui::EndChild();
                }

                ImGui::SameLine();

                if (ImGui::Button("Clear All")) {
                    playLayer::macro_sqp.clear();
                }
            }

            if (item_current_idx == 5) {
                if (ImGui::Checkbox("Noclip", &nolcip)) {
                    if (nolcip) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A23C), "\xE9\x79\x06\x00\x00", 5, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A23C), "\x6A\x14\x8B\xCB\xFF", 5, NULL);
                    }
                }

                if (ImGui::Checkbox("Practice Music Hack", &practice_music_hack)) {
                    if (practice_music_hack) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C925), "\x90\x90\x90\x90\x90\x90", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D143), "\x90\x90", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A563), "\x90\x90", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A595), "\x90\x90", 2, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C925), "\x0F\x85\xF7\x00\x00\x00", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D143), "\x75\x41", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A563), "\x75\x3E", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A595), "\x75\x0C", 2, NULL);
                    }
                }

                if (ImGui::Checkbox("Ignore ESC", &ignore_esc)) {
                    if (ignore_esc) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1E644C), "\x90\x90\x90\x90\x90", 5, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1E644C), "\xE8\xBF\x73\x02\x00", 5, NULL);
                    }
                }

                if (ImGui::Checkbox("No Respawn Flash", &no_respawn_flash)) {
                    if (no_respawn_flash) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1EF36D), "\xE9\xA8\x00\x00\x00\x90", 6, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1EF36D), "\x0F\x85\xA7\x00\x00\x00", 6, NULL);
                    }
                }

                if (ImGui::Checkbox("Disable Death Effects", &disable_death_effects)) {
                    if (disable_death_effects) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1EFBA4), "\x90\x90\x90\x90\x90", 5, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1EFBA4), "\xE8\x37\x00\x00\x00", 5, NULL);
                    }
                }

                if (ImGui::Checkbox("Practice Coins", &practice_coins)) {
                    if (practice_coins) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x204F10), "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 13, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x204F10), "\x80\xBE\x95\x04\x00\x00\x00\x0F\x85\xDE\x00\x00\x00", 13, NULL);
                    }
                }

                if (ImGui::Checkbox("Anticheat Bypass", &anticheat_bypass)) {
                    if (anticheat_bypass) {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x202AAA), "\xEB\x2E", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x15FC2E), "\xEB", 1, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D3B3), "\x90\x90\x90\x90\x90", 5, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FF7A2), "\x90\x90", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x18B2B4), "\xB0\x01", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C4E6), "\xE9\xD7\x00\x00\x00\x90", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD557), "\xEB\x0C", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD742), "\xC7\x87\xE0\x02\x00\x00\x01\x00\x00\x00\xC7\x87\xE4\x02\x00\x00\x00\x00\x00\x00\x90\x90\x90\x90\x90\x90", 26, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD756), "\x90\x90\x90\x90\x90\x90", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD79A), "\x90\x90\x90\x90\x90\x90", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD7AF), "\x90\x90\x90\x90\x90\x90", 6, NULL);
                    }
                    else {
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x202AAA), "\x74\x2E", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x15FC2E), "\x74", 1, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D3B3), "\xE8\x58\x04\x00\x00", 5, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FF7A2), "\x74\x6E", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x18B2B4), "\x88\xD8", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C4E6), "\x0F\x85\xD6\x00\x00\x00", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD557), "\x74\x0C", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD742), "\x80\xBF\xDD\x02\x00\x00\x00\x0F\x85\x0A\xFE\xFF\xFF\x80\xBF\x34\x05\x00\x00\x00\x0F\x84\xFD\xFD\xFF\xFF", 26, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD557), "\x74\x0C", 2, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD756), "\x0F\x84\xFD\xFD\xFF\xFF", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD79A), "\x0F\x84\xB9\xFD\xFF\xFF", 6, NULL);
                        WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD7AF), "\x0F\x85\xA4\xFD\xFF\xFF", 6, NULL);
                    }
                }
            }

            if (item_current_idx == 6) {
                ImGui::Text("DashReplay GUI v2.3.0b");
                ImGui::Text("DashReplay Engine v3.2.0");
                ImGui::Text("DashReplay created by TobyAdd, Powered by Dear ImGui");
                ImGui::Separator();
                ImGui::Text("Rigth/Left Alt - Toggle UI");
                ImGui::Text("C - Enable Frame Advance + Next Frame");
                ImGui::Text("F - Disable Frame Advance");
                ImGui::Text("P - Toggle Playback");
                ImGui::Text("S - Spam Bot Toggle");
                ImGui::Separator();
                ImGui::Text("Special Thanks:");
                ImGui::Text("HJfod Absolute Adaf Eimaen Ubuntu Matcool qb");
                ImGui::Text("Everyone who tested DashReplay");
                ImGui::Text("And DashReplay Community");
                ImGui::Separator();
                if (ImGui::MenuItem("Discord Server")) ShellExecuteA(0, "open", "https://discord.com/invite/mQHXzG72vU", 0, 0, SW_SHOWNORMAL);

            }
        }

        ImGui::End();
    }
}

inline void(__thiscall* dispatchKeyboardMSG)(void* self, int key, bool down);
void __fastcall dispatchKeyboardMSGHook(void* self, void*, int key, bool down) {
	dispatchKeyboardMSG(self, key, down);
    auto pl = gd::GameManager::sharedState()->getPlayLayer();
    if (down && key == 18) {
        show = !show;
    }

    if (pl && down && key == 'C') {
        FPSMultiplier::frame_advance = true;
        FPSMultiplier::nextframe = true;
    }

    if (pl && down && key == 'F') {
        FPSMultiplier::frame_advance = false;
        FPSMultiplier::nextframe = false;
    }

    if (pl && down && key == 'S') {
        spambot::enable = !spambot::enable;
    }

    if (down && key == 'P') {
        if (playLayer::mode != 2) playLayer::mode = 2;
        else playLayer::mode = 0;
    }

}

DWORD WINAPI Main(void* hModule) {
    srand((uint32_t)time(NULL));
    if (!std::filesystem::is_directory(".DashReplay") || !std::filesystem::exists(".DashReplay"))
		std::filesystem::create_directory(".DashReplay");
        
    ImGuiHook::Load(RenderMain);
	MH_Initialize();
    FPSMultiplier::Setup();
    playLayer::mem_init();
	MH_CreateHook(
		(PVOID)(GetProcAddress(GetModuleHandleA("libcocos2d.dll"), "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z")),
		dispatchKeyboardMSGHook,
		(LPVOID*)&dispatchKeyboardMSG
    );
	MH_EnableHook(MH_ALL_HOOKS);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
        CreateThread(0, 0x1000, Main, hModule, 0, 0);
        break;
    }
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}