#include "clicks.h"
#include "replayEngine.h"

bool select = true;

namespace clicks
{
    char clickpack[128] = "";
    extern char output[128] = "DashReplay/Clicks/Output.mp3";
    bool include_clicks = false;
    bool softclicks = false;
    int softdelay = 200;

    bool hardclicks = false;
    int harddelay = 500;

    bool do_clicks(bool msg)
    {
        if (filesystem::exists("ffmpeg.exe"))
        {
            if (filesystem::exists("clicks.exe"))
            {
                string old_replay = (string)dashreplay::info::replay_name;
                strcpy_s(dashreplay::info::replay_name, string("temp_replay_for_clicks").c_str());
                if (dashreplay::replay::save())
                {
                    string cmd_line = "clicks.exe ";
                    cmd_line += "-i DashReplay/Replays/temp_replay_for_clicks.json ";
                    cmd_line += "-o \"" + (string)output + "\" ";
                    cmd_line += "--clickpack \"" + (string)clickpack + "\" ";
                    cmd_line += "--end-delay 3 --mp3-export ";
                    if (softclicks)
                        cmd_line += "--softclicks ";
                    if (hardclicks)
                        cmd_line += "--hardclicks ";
                    if (softclicks)
                        cmd_line += "--softclick-delay " + to_string(softdelay) + " ";
                    if (hardclicks)
                        cmd_line += "--hardclick-delay " + to_string(softdelay) + " ";
                    cmd_line += "--type dashreplay";
                    Console::WriteLine("executing: " + cmd_line);
                    auto process = subprocess::Popen(cmd_line);
                    if (process.close())
                    {
                        if (msg)
                            gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Something went wrong... Check console")->show();
                        return false;
                    }
                    else
                    {
                        if (msg)
                            gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Done!")->show();                        
                    }
                    
                }
                else
                {
                    if (msg)
                        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay doesn't have actions")->show();
                    return false;
                }
                filesystem::remove("DashReplay/Replays/temp_replay_for_clicks.json");
                strcpy_s(dashreplay::info::replay_name, old_replay.c_str());
                return true;                
            }
            else
            {
                if (msg)
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "TCB++ doesn't exist")->show();
                return false;
            }
        }
        else
        {
            if (msg)
                gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "FFmpeg doesn't exist")->show();
            return false;
        }
        return false;
    }

    void render()
    {
        ImGui::Text("Clickpack Folder:");
        if (!select) {
            ImGui::InputText("##clickpack_name", clickpack, IM_ARRAYSIZE(clickpack));
            ImGui::SameLine();
            if (ImGui::ArrowButton("##comboopen", ImGuiDir_Down)) select = true;
        }
        else {
            for (const auto & entry: filesystem::directory_iterator("DashReplay/Clicks")) {
                string click = entry.path().filename().string();
                if (filesystem::is_directory("DashReplay/Clicks/" + click)) {
                    if (ImGui::MenuItem(click.c_str())) {
                        strcpy_s(clickpack, string("DashReplay/Clicks/" + click).c_str());
                        select = false;
                    }
                }
            }
        }
        
        ImGui::Checkbox("Softclicks", &softclicks);
        ImGui::SameLine();
        ImGui::PushItemWidth(160.f);
        ImGui::InputInt("Delay##1", &softdelay);
        ImGui::Checkbox("Hardclicks", &hardclicks);
        ImGui::SameLine();
        ImGui::PushItemWidth(160.f);
        ImGui::InputInt("Delay##2", &harddelay);
        ImGui::Text("Output file:");
        ImGui::PushItemWidth(300.f);
        ImGui::InputText("##output_name", output, IM_ARRAYSIZE(output));
        if (ImGui::Button("Render"))
        {
            if (((clickpack != NULL) && (clickpack[0] == '\0'))) {
                gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Clickpack not selected")->show();
            }
            else {
                do_clicks(true);
            }            
        }

        ImGui::Separator();
        ImGui::Checkbox("Include clicks on recording", &include_clicks);
        ImGui::Separator();
        ImGui::Text("[?] clickpack folder structure");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(structure.c_str());
    }
}