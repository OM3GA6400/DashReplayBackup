#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include "pch.h"
#include <imgui-hook.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include "replayEngine.h"
#include "framerate.h"
#include "playLayer.h"
#include "speedhackAudio.h"
#include "hooks.h"
#include "spambot.h"
#include "editor.h"
#include "clicks.h"
#include "hacks.h"

bool show = true;
bool inited = false;
bool windowRecordOverwrite = true;
static float fadeTimer = 4.0f;

char* items[] = {"General", "Assist", "Editor", "Recorder", "Sequence", "Converter", "Clickbot", "Hacks", "About"};
char* converterTypes[] = {"Plain Text (.txt)"};
int index = 0;

bool opennedSP = false;
vector<string> replay_list;


void SelectReplay() {
    auto itemx = ImGui::GetItemRectMin().x;
    auto itemy = ImGui::GetItemRectMax().y;
    auto itemw = ImGui::GetItemRectSize().x;
    ImGui::SameLine(0);
    if (ImGui::ArrowButton("##comboopen", opennedSP ? ImGuiDir_Up : ImGuiDir_Down))  {
        opennedSP = !opennedSP; 
        if (opennedSP) {
            replay_list.clear();
            for (const auto & entry : filesystem::directory_iterator("DashReplay/Replays")) {
                string replay = entry.path().filename().string();
                if (replay.find(".json") != std::string::npos) {
                    replay_list.push_back(entry.path().filename().string().erase(replay.size()-5, replay.size()));
                }
            }
        }
    }   
    if (opennedSP) {
        ImGui::SetNextWindowPos(ImVec2(itemx, itemy + 4));
        ImGui::SetNextWindowSize(ImVec2(itemw + ImGui::GetItemRectSize().x, NULL));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
        ImGui::Begin("##replaylist", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        for (int i = 0; i < (int)replay_list.size(); i++) {
            if (ImGui::MenuItem(replay_list[i].c_str())) {
                strcpy_s(dashreplay::info::replay_name, replay_list[i].c_str());
                opennedSP = false;
            }
        }            
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

void RenderMain() {
    if (show) {
        if (!inited) {
            ImGui::SetNextWindowSize(ImVec2(600, 400));
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            inited = true;
        }
        
        ImGui::Begin("DashReplay", &show, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::BeginChild("##leftside", ImVec2(150, NULL));
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (index == n);
            if (ImGui::Selectable(items[n], is_selected)) index = n;
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0, 5);

        ImGui::BeginChild("##rightside", ImVec2(NULL, NULL));   

        if (index == 0) {
            ImGui::RadioButton("Disable", &dashreplay::info::mode, state::disable); ImGui::SameLine();
            if (ImGui::RadioButton("Record", &dashreplay::info::mode, state::record)) {
                dashreplay::replay::p1.clear();
                dashreplay::replay::p2.clear();
                if (gd::GameManager::sharedState()->getGameVariable("0027")) { //Disabling Auto-Checkpoints option
                    gd::GameManager::sharedState()->setGameVariable("0027", false); 
                }                 
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("Play", &dashreplay::info::mode, state::play)) {

            }
            ImGui::Separator();
            ImGui::InputText("##replay", dashreplay::info::replay_name, IM_ARRAYSIZE(dashreplay::info::replay_name));
            SelectReplay();

            if (ImGui::Button("Save", ImVec2(80, NULL))) {
                if (((dashreplay::info::replay_name != NULL) && (dashreplay::info::replay_name[0] == '\0'))) 
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay name is empty")->show(); 
                else {
                    if (dashreplay::replay::save()) gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay saved")->show();
                    else gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay doesn't have actions")->show();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Load", ImVec2(80, NULL))) {
                if (dashreplay::replay::load()) {
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay loaded")->show(); 
                }
                else {
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay doesn't exist")->show(); 
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear Replay", ImVec2(108, NULL))) {
                dashreplay::replay::p1.clear();
                dashreplay::replay::p2.clear();
            }

            ImGui::Separator();

            ImGui::PushItemWidth(135.f);
            ImGui::DragFloat("##FPS", &dashreplay::info::fps, 1.f, 1.f, FLT_MAX, "FPS: %.2f");

            ImGui::SameLine();

            ImGui::PushItemWidth(137.f);
            if (ImGui::DragFloat("##Speed", &dashreplay::info::speedhack, 0.01f, 0.f, FLT_MAX, "Speed: %.2f")) {
                if (dashreplay::advanced::speedhack_audio) speedhack_audio::set(dashreplay::info::speedhack);
            }

            ImGui::Separator();

            ImGui::Checkbox("Practice Fix (Recommended)", &dashreplay::fixes::practice_fix);
            ImGui::SameLine();
            ImGui::Checkbox("Accuracy Fix", &dashreplay::fixes::accuracy_fix);

            ImGui::Separator();

            ImGui::Checkbox("Ignore Inputs on Playing", &dashreplay::advanced::ignore_input);
            ImGui::SameLine();

            if (ImGui::Checkbox("Speedhack Audio", &dashreplay::advanced::speedhack_audio)) {
                if (dashreplay::advanced::speedhack_audio)  {
                    speedhack_audio::set(dashreplay::info::speedhack);
                }
                else {
                    speedhack_audio::set(1.f);
                }
            }

            ImGui::Separator();

            ImGui::Text("Frame %i", dashreplay::frame::get_frame());
        }  
        else if (index == 1) {
            ImGui::Checkbox("Frame Advance", &dashreplay::frameadvance::enabled);
            ImGui::Separator();
            ImGui::Checkbox("Auto-Clicker", &spambot::enable);
            
            ImGui::SameLine();
            ImGui::PushItemWidth(100.f);	
            ImGui::DragInt("##spamhold", &spambot::push, 1, 1, INT_MAX, "Hold: %i");

            ImGui::SameLine();
            ImGui::PushItemWidth(100.f);	
            ImGui::DragInt("##spamreelase", &spambot::release, 1, 1, INT_MAX, "Release: %i");

            ImGui::Checkbox("Player 1", &spambot::player1);
            ImGui::SameLine();
            ImGui::Checkbox("Player 2", &spambot::player2);

            ImGui::Separator();     

            ImGui::Checkbox("Dual Clicks", &dashreplay::advanced::dual_clicks); 
        }  
        else if (index == 2) {
            editor::render();
        }
        else if (index == 3) {
            ImGui::Text("Output Name:");
            ImGui::InputText("##video_name", dashreplay::irecorder::video_name, IM_ARRAYSIZE(dashreplay::irecorder::video_name));
            ImGui::SameLine();
            if (ImGui::Button(dashreplay::irecorder::recorder_c ? "Recording..." : "Record")) {
                if ((dashreplay::irecorder::video_name != NULL) && (dashreplay::irecorder::video_name[0] == '\0')) {
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Invalid output name")->show();
                }
                else {
                    if (!gd::GameManager::sharedState()->getPlayLayer()) {
                        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "You are not in the level")->show();
                    }
                    else {
                        if (clicks::include_clicks) {
                            if (((clicks::clickpack != NULL) && (clicks::clickpack[0] == '\0'))) {
                                gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Important! Clickpack not selected, the clicks will not be included in video. Go to \"Clickbot\" tab and select clickpack.")->show();
                            }
                        }
                        if (std::filesystem::exists("ffmpeg.exe")) {
                            dashreplay::irecorder::recorder_c = !dashreplay::irecorder::recorder_c;
                            if (dashreplay::irecorder::recorder_c) {
                                dashreplay::info::mode = state::play;
                                dashreplay::irecorder::recorder.start("DashReplay/Videos/" + (string)dashreplay::irecorder::video_name);
                            }
                            else {
                                if (dashreplay::irecorder::recorder.m_recording)
                                    dashreplay::irecorder::recorder.stop();
                            }
                        }
                        else {
                            gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "FFmpeg not found")->show();
                        } 
                    }
                }
            }
            ImGui::Separator();

            int width = (int)dashreplay::irecorder::recorder.m_width;
            ImGui::PushItemWidth(40.f);
            if (ImGui::InputInt("##ir_width", &width, 0)) {
                dashreplay::irecorder::recorder.m_width = width;
            }

            ImGui::SameLine();
            ImGui::Text("x");
            ImGui::SameLine();

            int height = (int)dashreplay::irecorder::recorder.m_height;
            ImGui::PushItemWidth(40.f);
            if (ImGui::InputInt("##ir_height", &height, 0)) {
                dashreplay::irecorder::recorder.m_height = height;
            }

            ImGui::SameLine();
            ImGui::Text("@");
            ImGui::SameLine();

            int fps = (int)dashreplay::irecorder::recorder.m_fps;
            ImGui::PushItemWidth(40.f);
            if (ImGui::InputInt("FPS##ir_fps", &fps, 0)) {
                dashreplay::irecorder::recorder.m_fps = fps;
            }
            
            char bitrate[128];
            strcpy_s(bitrate, dashreplay::irecorder::recorder.m_bitrate.c_str());
            ImGui::PushItemWidth(50.f);
            if (ImGui::InputText("Bitrate##ir_birtate", bitrate, IM_ARRAYSIZE(bitrate))) {
                dashreplay::irecorder::recorder.m_bitrate = (string)bitrate;
            }

            ImGui::SameLine();

            char codec[128];
            strcpy_s(codec, dashreplay::irecorder::recorder.m_codec.c_str());
            ImGui::PushItemWidth(60.f);
            if (ImGui::InputText("Codec##ir_codec", codec, IM_ARRAYSIZE(codec))) {
                dashreplay::irecorder::recorder.m_codec = (string)codec;
            }

            ImGui::Separator();

            ImGui::Text("Extra arguments:");

            char args[128];
            strcpy_s(args, dashreplay::irecorder::recorder.m_extra_args.c_str());
            ImGui::PushItemWidth(350.f);
            if (ImGui::InputText("##ir_args", args, IM_ARRAYSIZE(args))) {
                dashreplay::irecorder::recorder.m_extra_args = (string)args;
            }

            ImGui::Separator();

            ImGui::Text("Extra audio arguments:");

            char args_a[128];
            strcpy_s(args_a, dashreplay::irecorder::recorder.m_extra_audio_args.c_str());
            ImGui::PushItemWidth(350.f);
            if (ImGui::InputText("##ir_args_a", args_a, IM_ARRAYSIZE(args_a))) {
                dashreplay::irecorder::recorder.m_extra_audio_args = (string)args_a;
            }

            ImGui::Separator();

            if (ImGui::Button("HD")) {
                dashreplay::irecorder::recorder.m_width = 1280;
                dashreplay::irecorder::recorder.m_height = 720;
                dashreplay::irecorder::recorder.m_fps = 60;
                dashreplay::irecorder::recorder.m_bitrate = "15M";
            }

            ImGui::SameLine();

            if (ImGui::Button("FULL HD")) {
                dashreplay::irecorder::recorder.m_width = 1920;
                dashreplay::irecorder::recorder.m_height = 1080;
                dashreplay::irecorder::recorder.m_fps = 60;
                dashreplay::irecorder::recorder.m_bitrate = "50M";
            }

            ImGui::SameLine();

            if (ImGui::Button("4K")) {
                dashreplay::irecorder::recorder.m_width = 3840;
                dashreplay::irecorder::recorder.m_height = 2160;
                dashreplay::irecorder::recorder.m_fps = 60;
                dashreplay::irecorder::recorder.m_bitrate = "70M";
            }
        }
        else if (index == 4) {
            ImGui::Checkbox("Toggle", &dashreplay::sequence::enable_sqp);
            ImGui::SameLine();
            ImGui::Checkbox("Random Sequence", &dashreplay::sequence::random_sqp);
            if (!dashreplay::sequence::replays.empty()) {
                ImGui::SameLine(NULL, 50);            
                ImGui::Text("Current Replay: %s", dashreplay::sequence::replays[dashreplay::sequence::current_idx].c_str());
            }
            ImGui::PushItemWidth(250.f);
            ImGui::InputText("##sequence_input", dashreplay::sequence::replay_sq_name, IM_ARRAYSIZE(dashreplay::sequence::replay_sq_name));
            ImGui::SameLine();

            if (ImGui::Button("Add")) {
                dashreplay::sequence::replays.push_back((string)dashreplay::sequence::replay_sq_name);
                dashreplay::sequence::first_sqp = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Remove")) {
                if (dashreplay::sequence::replays.size() > (size_t)dashreplay::sequence::current_idx) {
                    dashreplay::sequence::replays.erase(dashreplay::sequence::replays.begin()+dashreplay::sequence::current_idx);
                    dashreplay::sequence::first_sqp = true;
                }                    
            }

            ImGui::SameLine();

            if (ImGui::Button("Remove All")) {
                dashreplay::sequence::replays.clear();
                dashreplay::sequence::first_sqp = true;
            }

            if (ImGui::BeginChild("##sqp_panel", ImVec2(NULL, NULL), true))
            {	
                for (size_t n = 0; n < dashreplay::sequence::replays.size(); n++)
                {
                    bool is_selected = (dashreplay::sequence::current_idx == n);
                    string anticonflict = dashreplay::sequence::replays[n] + "##" + to_string(n);
                    if (ImGui::Selectable(anticonflict.c_str(), is_selected)) dashreplay::sequence::current_idx = n;
                }
                ImGui::EndChild();
            }
        }
        else if (index == 5) {
            ImGui::Combo("##ConverterType", &dashreplay::converter::converterType, converterTypes, IM_ARRAYSIZE(converterTypes));
            if (ImGui::Button("Convert")) {
                dashreplay::converter::convert();
            }
            ImGui::SameLine();
            if (ImGui::Button("Matcool Converter")) {
                ShellExecuteA(0, "open", "https://matcool.github.io/gd-macro-converter/", 0, 0, SW_SHOWNORMAL);
            }
            if (dashreplay::converter::converterType == 0) {
                ImGui::Text("Replay will be saved to \"DashReplay/converted.txt\"");
            }
        }
        else if (index == 6) {
            clicks::render();
        }
        else if (index == 7) {
            hacks::render();
        }
        else if (index == 8) {
            ImGui::Text("DashReplay GUI v4.0.0b");
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
            ImGui::Text("HJfod, Adaf - Help in early days"); //Absolute  Eimaen Ubuntu Matcool qb
            ImGui::Text("Absolute - Hacks");
            ImGui::Text("Eimaen, howhathe - Some coding stuff");
            ImGui::Text("Ubuntu - Font");
            ImGui::Text("Matcool - Internal Recorder");
            ImGui::Text("Acid - Clickbot");
            ImGui::Text("And DashReplay Community");
            ImGui::Separator();
            if (ImGui::MenuItem("Discord Server")) ShellExecuteA(0, "open", "https://discord.com/invite/mQHXzG72vU", 0, 0, SW_SHOWNORMAL);
            if (ImGui::MenuItem("Github")) ShellExecuteA(0, "open", "https://github.com/TobyAdd/DashReplay", 0, 0, SW_SHOWNORMAL);
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

void CreateDir() {
    if (!std::filesystem::is_directory("DashReplay") || !std::filesystem::exists("DashReplay")) {
        std::filesystem::create_directory("DashReplay");
        std::filesystem::create_directory("DashReplay/Replays");
        std::filesystem::create_directory("DashReplay/Videos");
        std::filesystem::create_directory("DashReplay/Clicks");
    }
}

DWORD WINAPI ThreadMain(void* hModule) {
    srand((unsigned)time(NULL));
    Console::Unlock();
    memory::get_hwnd();
    if (!IsWindows81orHigher()) {
        MessageBoxA(memory::window, "DashReplay requires Windows 8.1 or higher. Sorry :(", "Something went wrong ~(>_<~)", MB_OK | MB_ICONERROR);
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    }
    hacks::anticheat_bypass_f(true);
    CreateDir();
    ImGuiHook::setRenderFunction(RenderMain);
    ImGuiHook::setToggleCallback([]() {
        show = !show;
    });
    if (MH_Initialize() == MH_OK) {
        ImGuiHook::setupHooks([](void* target, void* hook, void** trampoline) {
            MH_CreateHook(target, hook, trampoline);
        });
        framerate::mem_init();
        playLayer::mem_init();
        speedhack_audio::mem_init();
        hooks::mem_init();
        MH_EnableHook(MH_ALL_HOOKS);
    } else {
        MessageBoxA(memory::window, "Looks like minhook failed to initialize! DashReplay will not be loaded", "Something went wrong ~(>_<~)", MB_OK | MB_ICONERROR);
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    }
    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x1000, ThreadMain, hModule, 0, 0);
    }
    return TRUE;
}

