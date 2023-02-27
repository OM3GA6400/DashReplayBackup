#include "clicks.h"
#include "replayEngine.h"

bool select = true;

namespace clicks
{
    vector<string> clickpacks;
    char output[128] = "DashReplay\\Clicks\\Output.mp3";
    bool include_clicks = false;
    bool softclicks = false;
    int softdelay = 200;
    int selected_clickpack_idx = 0;

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
                    cmd_line += "-i DashReplay\\Replays\\temp_replay_for_clicks.json ";
                    cmd_line += "-o \"" + (string)output + "\" ";
                    cmd_line += "--clickpack \"" + clickpacks[selected_clickpack_idx] + "\" ";
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
                filesystem::remove("DashReplay\\Replays\\temp_replay_for_clicks.json");
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
        if (clickpacks.empty()) {
            ImGui::Text("Where clickpacks!?");
        }
        else {
            ImGui::Text("Clickpack Folder:");
            ImGui::BeginChild("##clickpackchild", ImVec2(NULL, 120), true);
            size_t n = 0;
            for (size_t i = 0; i < clickpacks.size(); i++) {
                bool is_selected = (selected_clickpack_idx == n);
                if (ImGui::Selectable(clickpacks[selected_clickpack_idx].c_str(), is_selected)) selected_clickpack_idx = n;
            }
            ImGui::EndChild();
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
                if (clickpacks.empty()) {
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
        if (ImGui::Button("Update Clickpack List")) {
            update_list();
        }
    }

    void update_list() {
        clickpacks.clear();
        for (const auto & entry: filesystem::directory_iterator("DashReplay\\Clicks")) {
            string click = entry.path().string();
            if (filesystem::is_directory(click)) {
                clickpacks.push_back(click);
            }            
        }
    }
}