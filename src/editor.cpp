#include <imgui.h>
#include "replayEngine.h"

bool player_for_editing = true;
int editor_index = 0;
char replay[128];

namespace editor {
    void render() {
        if (!dashreplay::replay::p1.empty()) { 
            auto size = ImGui::GetWindowSize();
            auto pos = ImGui::GetWindowPos();
            ImGui::SetNextWindowPos(ImVec2(size.x + pos.x + 20, pos.y - 30));
            ImGui::Begin("Editor", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);  
            if (ImGui::Button("Add Action")) {
                if (player_for_editing) {
                    replay_data emptydata = {(unsigned)editor_index + 1};
                    dashreplay::replay::p1.insert(dashreplay::replay::p1.begin()+editor_index+1, emptydata);
                    editor_index++;
                }
                else {
                    replay_data emptydata = {(unsigned)editor_index + 1};
                    dashreplay::replay::p2.insert(dashreplay::replay::p2.begin()+editor_index+1, emptydata);
                    editor_index++;
                }
            }   

            ImGui::SameLine();

            if (ImGui::Button("Remove Action")) {

                if ((player_for_editing ? (int)dashreplay::replay::p1.size() : (int)dashreplay::replay::p2.size()) > editor_index) {
                    if (player_for_editing) {
                        dashreplay::replay::p1.erase(dashreplay::replay::p1.begin() + editor_index);
                    }
                    else {
                        dashreplay::replay::p2.erase(dashreplay::replay::p2.begin() + editor_index);
                    }
                }
            }   
            
            ImGui::Separator();

            ImGui::PushItemWidth(170.f);
            int frame = player_for_editing ? (int)dashreplay::replay::p1[editor_index].frame : (int)dashreplay::replay::p2[editor_index].frame;
            if (ImGui::DragInt("##frame_edit", &frame, 1.f, 1, INT_MAX, "Frame = %i")) {
                if (player_for_editing)
                    dashreplay::replay::p1[editor_index].frame = frame;
                else
                    dashreplay::replay::p2[editor_index].frame = frame;
            }       

            ImGui::PushItemWidth(170.f);
            ImGui::DragFloat("##xpos_edit", player_for_editing ? &dashreplay::replay::p1[editor_index].x_pos : &dashreplay::replay::p2[editor_index].x_pos, 0.000001f, 0, FLT_MAX, "Position X = %f");

            ImGui::PushItemWidth(170.f);
            ImGui::DragFloat("##ypos_edit", player_for_editing ? &dashreplay::replay::p1[editor_index].y_pos : &dashreplay::replay::p2[editor_index].y_pos, 0.000001f, 0, FLT_MAX, "Position Y = %f");
            
            ImGui::PushItemWidth(170.f);
            float y_vel = player_for_editing ? (float)dashreplay::replay::p1[editor_index].y_vel : (float)dashreplay::replay::p2[editor_index].y_vel;
            if (ImGui::DragFloat("##yaccel_edit", &y_vel, 0.000001f, 0, FLT_MAX, "Y Vel = %f")) {
                if (player_for_editing)
                    dashreplay::replay::p1[editor_index].y_vel = y_vel;
                else
                    dashreplay::replay::p2[editor_index].y_vel = y_vel;
            }            

            ImGui::Checkbox(dashreplay::replay::p1[editor_index].down ? "Hold" : "Release", player_for_editing ? &dashreplay::replay::p1[editor_index].down : &dashreplay::replay::p2[editor_index].down);

            ImGui::Separator();

            ImGui::Checkbox(player_for_editing ? "Player 1" : "Player 2", &player_for_editing);
            ImGui::End();     

            ImGui::BeginChild("##editor_child", ImVec2(NULL, NULL));
                if (ImGui::BeginTable("##editor_table", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiWindowFlags_NoResize))
                {
                    ImGui::TableNextColumn(); ImGui::TableHeader("Frame");
                    ImGui::TableNextColumn(); ImGui::TableHeader("X Position");
                    ImGui::TableNextColumn(); ImGui::TableHeader("Y Position");
                    ImGui::TableNextColumn(); ImGui::TableHeader("Y Acceleration");
                    ImGui::TableNextColumn(); ImGui::TableHeader("Action");
                    for (size_t i = 0; i < (player_for_editing ? dashreplay::replay::p1.size() : dashreplay::replay::p2.size()); i++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        bool is_selected = (editor_index == i);
                        string useless = to_string(player_for_editing ? dashreplay::replay::p1[i].frame : dashreplay::replay::p2[i].frame) + "##" + to_string(i);
                        if (ImGui::Selectable(useless.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {                            
                            editor_index = i;
                        }                           
                        ImGui::TableNextColumn();
                        ImGui::Text(to_string(player_for_editing ? dashreplay::replay::p1[i].x_pos : dashreplay::replay::p2[i].x_pos).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(to_string(player_for_editing ? dashreplay::replay::p1[i].y_pos : dashreplay::replay::p2[i].y_pos).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(to_string(player_for_editing ? dashreplay::replay::p1[i].y_vel : dashreplay::replay::p2[i].y_vel).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(player_for_editing ? (dashreplay::replay::p1[i].down ? "Hold" : "Release") : (dashreplay::replay::p2[i].down ? "Hold" : "Release"));                    
                    }
                    ImGui::EndTable();
                }
            ImGui::EndChild();
        }
        else {
            ImGui::Text("Replay doesn't have actions!");
            if (ImGui::Button("Add Action")) {
                replay_data newdata = {1};
                dashreplay::replay::p1.push_back(newdata);
            }
        }
    }
}