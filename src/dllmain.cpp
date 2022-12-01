#include <windows.h>
#include <shellapi.h>
#include <cocos2d.h>
#include <gd.h>
#include "console.h"
#include "fpsBypass.h"
#include "playLayer.h"
#include "spamBot.h"
#include <imgui-hook.hpp>
#include <imgui_internal.h>
#include <imgui.h>
#include <ImGuiFileDialog.h>

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

const char* converterTypes[]{"Plain Text (.txt)"};
int converterType = 0;

const char* items[] = { "General", "Assist", "Editor", "Clicks", "Converter", "Hacks", "About"};
int item_current_idx = 0;

int replay_select_player_p1 = 1;
int replay_current = 0;

string OpenFileDialog(LPCSTR filter) {
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lpstrFilter = filter;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    if (GetOpenFileName(&ofn)) return fileName;
    return "";
}

string SaveFileDialog(LPCSTR filter) {
    OPENFILENAME sfd;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&sfd, sizeof(sfd));
    sfd.lpstrFilter = filter;
    sfd.lStructSize = sizeof(OPENFILENAME);
    sfd.lpstrFile = fileName;
    sfd.nMaxFile = MAX_PATH;
    if (GetSaveFileName(&sfd)) return fileName;
    return "";
}

void RenderMain() {
    if (show) {
        if (ImGuiFileDialog::Instance()->Display("ChooseReplay")) 
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                playLayer::clearMacro();
                playLayer::loadReplay(ImGuiFileDialog::Instance()->GetFilePathName());
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveReplay")) 
        {
            if (ImGuiFileDialog::Instance()->IsOpened())
            {
                playLayer::saveReplay(ImGuiFileDialog::Instance()->GetFilePathName());
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("ImportCustomMacro")) 
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                playLayer::clearMacro();

                string line;
                fstream file(ImGuiFileDialog::Instance()->GetFilePathName());

                vector<string> splitwords;
                string splitword;

                getline(file, line);
                FPSMultiplier::g_target_fps = stof(line);
                while (getline(file, line)) {
                    splitwords.clear();
                    if (!line.empty()) {
                        istringstream splitstr(line);
                        while (getline(splitstr, splitword, ' ')) {
                            splitwords.push_back(splitword);
                        }
                        int framer = stoi(splitwords[0]);
                        int p1 = stoi(splitwords[1]);
                        int p2 = stoi(splitwords[2]);
                        Console::Write("Checking " + to_string(framer) + " frame\n");
                        while ((int)playLayer::replay_p1.size() != framer) {
                            replaydata emptydata_p1 = {-1, -1, -1, -1, -1, -1};
                            playLayer::replay_p1.push_back(emptydata_p1);
                        }
                        
                        replaydata newdata_p1 = {framer, -1, -1 , -1, -1, p1};
                        playLayer::replay_p1.push_back(newdata_p1);
                    }
                }
                file.close();
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveCustomMacro")) 
        {
            if (ImGuiFileDialog::Instance()->IsOpened())
            {
                if (converterType == 0) {
                    std::ofstream out(ImGuiFileDialog::Instance()->GetFilePathName());
                    out << FPSMultiplier::g_target_fps << "\n";
                    for (int i = 0; i < (int)playLayer::replay_p1.size(); i++) {
                        if (i == 0 || (playLayer::replay_p1[i].down == playLayer::replay_p1[i - 1].down && playLayer::replay_p2[i].down == playLayer::replay_p2[i - 1].down))
                            continue;
                        out << i << " " << playLayer::replay_p1[i].down << " " << playLayer::replay_p2[i].down << "\n";
                    }
                    out.close();	
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::Begin("DashReplay", &show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        if (!inited) {
            ImGui::SetWindowPos(ImVec2(10, 10));
            ImGui::SetWindowSize(ImVec2(500, 300));
            inited = true;
        }

        if (ImGui::BeginChild("##LeftSide", ImVec2(120, ImGui::GetContentRegionAvail().y), true))
        {	
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current_idx = n;

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

                if (ImGui::Button("Load", {60, NULL})) {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseReplay", "Choose Replay", ".*", "", 1, nullptr);
                    ImGuiFileDialog::Instance()->Display("ChooseReplay", ImGuiWindowFlags_NoCollapse, ImVec2(600,400)); 
                }

                ImGui::SameLine();

                if (ImGui::Button("Save", {60, NULL})) {           
                    ImGuiFileDialog::Instance()->OpenDialog("SaveReplay", "Save Replay", ".*", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
                    ImGuiFileDialog::Instance()->Display("SaveReplay", ImGuiWindowFlags_NoCollapse, ImVec2(600,400)); 
                }

                ImGui::SameLine();
                if (ImGui::Button("Clear Replay", {80, NULL})) {
                    playLayer::clearMacro();
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
                ImGui::Checkbox("FPS Bypass", &FPSMultiplier::fpsbypass_enabled);
                ImGui::SameLine();
                ImGui::Checkbox("FPS Multiplier", &FPSMultiplier::g_enabled);
                // ImGui::Separator();
                // ImGui::DragInt("##StartFrame", &playLayer::framestart, 1, 0, INT_MAX, "Start from %i frame");
                ImGui::Separator();
                ImGui::Text("Frame: %i", playLayer::frame);
                ImGui::Text("Replay Size: %i", (int)playLayer::replay_p1.size() + (int)playLayer::replay_p2.size());
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
                    for (int n = 0; n < (int)playLayer::replay_p1.size(); n++)
                    {
                        const bool is_selected = (replay_current == n);
                        ImGui::PushItemWidth(120.f);
                        if (ImGui::Selectable(to_string(playLayer::replay_p1[n].frame).c_str(), is_selected))
                            replay_current = n;

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
                    ImGui::EndChild();
                }
            }

            if (item_current_idx == 3) {
                ImGui::Text("Soon!");
            }

            if (item_current_idx == 4) {
                ImGui::Combo("##ConverterType", &converterType, converterTypes, IM_ARRAYSIZE(converterTypes));
                if (ImGui::Button("Convert")) {
                    if (converterType == 0) {
                        ImGuiFileDialog::Instance()->OpenDialog("SaveCustomMacro", "Save Custom Replay", ".txt", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
                        ImGuiFileDialog::Instance()->Display("SaveCustomMacro", ImGuiWindowFlags_NoCollapse, ImVec2(600,400)); 
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Import (Beta)")) {
                    if (converterType == 0) {
                        ImGuiFileDialog::Instance()->OpenDialog("ImportCustomMacro", "Choose Custom Macro", ".txt", "", 1, nullptr);
                        ImGuiFileDialog::Instance()->Display("ImportCustomMacro", ImGuiWindowFlags_NoCollapse, ImVec2(600,400)); 
                    }
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Supports only first Player. Note: DashReplay will be crashed if macro contains a duplicate of frames, you need to remove it yourself.");
                ImGui::SameLine();
                if (ImGui::Button("Matcool Converter")) {
                    ShellExecuteA(0, "open", "https://matcool.github.io/gd-macro-converter/", 0, 0, SW_SHOWNORMAL);
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
                ImGui::Text("DashReplay GUI v2.1.0b");
                ImGui::Text("DashReplay Engine v3.1.0");
                ImGui::Text("DashReplay created by TobyAdd, Powered by Dear ImGui");
                ImGui::Separator();
                ImGui::Text("Rigth/Left Alt - Toggle UI");
                ImGui::Text("C - Enable Frame Advance + Next Frame");
                ImGui::Text("F - Disable Frame Advance");
                ImGui::Text("P - Toggle Playback");
                ImGui::Text("S - Spam Bot Toggle");
                ImGui::Separator();
                ImGui::Text("Special Thanks:");
                ImGui::Text("HJfod Absolute Adaf Eimaen Ubuntu qb");
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
    if (down && key == 18) {
        show = !show;
    }

    if (gd::PlayLayer::get() && down && key == 'C') {
        FPSMultiplier::frame_advance = true;
        FPSMultiplier::nextframe = true;
    }

    if (gd::PlayLayer::get() && down && key == 'F') {
        FPSMultiplier::frame_advance = false;
        FPSMultiplier::nextframe = false;
    }

    if (gd::PlayLayer::get() && down && key == 'S') {
        spambot::enable = !spambot::enable;
    }

    if (down && key == 'P') {
        if (playLayer::mode != 2) playLayer::mode = 2;
        else playLayer::mode = 0;
    }

}

DWORD WINAPI Main(void* hModule) {
    Console::Write("Hello World\n");
    ImGuiHook::setRenderFunction(RenderMain);
	MH_Initialize();
    FPSMultiplier::Setup();
    playLayer::mem_init();
	ImGuiHook::setupHooks([](void* target, void* hook, void** trampoline) {
		MH_CreateHook(target, hook, trampoline);
	});
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