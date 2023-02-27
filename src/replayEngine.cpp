#include "replayEngine.h"
#include "playLayer.h"

namespace dashreplay {
    namespace replay {
        vector<replay_data> p1;
        vector<replay_data> p2;
        replay_data handle_recording(gd::PlayLayer* self, bool player) {
            replay_data newdata;
            unsigned frame = dashreplay::frame::get_frame();
            if (player) {
                newdata = {frame, self->m_player1->m_position.x, self->m_player1->m_position.y,
                    self->m_player1->m_yAccel, inputs::p1};
            }
            else {
                newdata = {frame, self->m_player2->m_position.x, self->m_player2->m_position.y,
                    self->m_player2->m_yAccel, inputs::p2};
            }
            return newdata;
        }

        void handle_playing(gd::PlayLayer* self) {
            unsigned frame = dashreplay::frame::get_frame();
            if (!dashreplay::replay::p1.empty() && frame < dashreplay::replay::p1.back().frame) {
                auto it = std::find_if(dashreplay::replay::p1.begin(), dashreplay::replay::p1.end(), [frame](const replay_data& a) { return a.frame == frame; });
                int index = it - dashreplay::replay::p1.begin();
                if (it != dashreplay::replay::p1.end()) {
                    if (fixes::accuracy_fix) {
                        self->m_player1->m_position.x = dashreplay::replay::p1[index].x_pos;
                        self->m_player1->m_position.y = dashreplay::replay::p1[index].y_pos;
                        self->m_player1->m_yAccel = dashreplay::replay::p1[index].y_vel;
                    }
                    if (dashreplay::replay::p1[index+1].down && !dashreplay::inputs::p1) {
                        playLayer::pushButton(self, 0, true);
                        dashreplay::inputs::p1 = true;
                    }
                    else if (!dashreplay::replay::p1[index+1].down && dashreplay::inputs::p1) {
                        playLayer::releaseButton(self, 0, true);
                        dashreplay::inputs::p1 = false;
                    }
                }

                auto it_ = std::find_if(dashreplay::replay::p2.begin(), dashreplay::replay::p2.end(), [frame](const replay_data& a) { return a.frame == frame; });
                int index_ = it_ - dashreplay::replay::p2.begin();
                if (it_ != dashreplay::replay::p2.end()) {
                    if (fixes::accuracy_fix) {
                        self->m_player2->m_position.x = dashreplay::replay::p2[index].x_pos;
                        self->m_player2->m_position.y = dashreplay::replay::p2[index].y_pos;
                        self->m_player2->m_yAccel = dashreplay::replay::p2[index].y_vel;
                    }
                    if (dashreplay::replay::p2[index+1].down && !dashreplay::inputs::p2) {
                        playLayer::pushButton(self, 0, false);
                        dashreplay::inputs::p2 = true;
                    }
                    else if (!dashreplay::replay::p2[index+1].down && dashreplay::inputs::p2) {
                        playLayer::releaseButton(self, 0, false);
                        dashreplay::inputs::p2 = false;
                    }
                }
            }
        }

        void handle_reset(gd::PlayLayer* self) {
            if (info::mode == state::record) {
                if (self->m_isPracticeMode && !practice_ex::p1.empty()) {
                    frame::frame_offset = practice_ex::p1.back().frame;              
                    unsigned frame = dashreplay::frame::get_frame();
                    while (replay::p1.back().frame > frame) {
                        replay::p1.pop_back();
                        replay::p2.pop_back();
                        if (replay::p1.empty()) break;
                        if (replay::p2.empty()) break;
                    }
                    if (dashreplay::fixes::practice_fix) practice_ex::fix(self);
                }
                else {
                    replay::p1.clear();
                    replay::p2.clear();
                    frame::frame_offset = 0;
                }
            }
            else if (info::mode == state::play) {
                playLayer::releaseButton(self, 0, true);
                playLayer::releaseButton(self, 0, false);
                inputs::p1 = false;
                inputs::p2 = false;
            }
        }

        bool save() {
            using namespace dashreplay::replay;
            if (p1.empty()) return false;
            json actions; 

            for (size_t i = 0; i < p1.size(); i++) 
                actions.push_back({{"frame", p1[i].frame}, {"x", p1[i].x_pos}, {"y", p1[i].y_pos}, {"a", p1[i].y_vel},
                    {"down", p1[i].down}, {"player", true}});

            for (size_t i = 0; i < p2.size(); i++)             
                actions.push_back({{"frame", p2[i].frame}, {"x", p2[i].x_pos}, {"y", p2[i].y_pos}, {"a", p2[i].y_vel},
                    {"down", p2[i].down}, {"player", false}});

            json replay = {
                {"fps", dashreplay::info::fps},
                {"actions", actions}
            };
            
            ofstream file("DashReplay/Replays/" + (string)info::replay_name + ".json");
            file << std::setw(4) << replay << std::endl;
            return true;
        }

        bool load() {
            ifstream file("DashReplay/Replays/" + (string)info::replay_name + ".json");
            if (file.is_open()) {
                p1.clear();
                p2.clear();
                using namespace dashreplay::replay;
                string file_content;
                getline(file, file_content, '\0');
                file.close();

                dashreplay::info::fps = json::parse(file_content)["fps"];

                json replay_json = json::parse(file_content)["actions"];
                for (size_t i = 0; i < replay_json.size(); i++) {
                    json replaydata_json = replay_json.at(i);
                    if (replaydata_json["player"]) {
                        replay_data newdata = {replaydata_json["frame"], replaydata_json["x"], replaydata_json["y"],
                            replaydata_json["a"], replaydata_json["down"]};
                        p1.push_back(newdata);
                    }
                    else {
                        replay_data newdata = {replaydata_json["frame"], replaydata_json["x"], replaydata_json["y"],
                            replaydata_json["a"], replaydata_json["down"]};
                        p2.push_back(newdata);                        
                    }
                    
                }
                return true;
            }
            else return false;
        }
    }   

    namespace practice_ex {
        vector<checkpoints_data> p1;
        vector<checkpoints_data> p2;
        void handle_checkpoint(gd::PlayLayer* self) {
            unsigned frame = frame::get_frame();
            checkpoints_data newdata_p1 = {frame, self->m_player1->m_position.x, self->m_player1->m_position.y, 
                self->m_player1->getRotation(), self->m_player1->m_xAccel, self->m_player1->m_yAccel, self->m_player1->m_jumpAccel,
                    self->m_player1->m_playerSpeed, self->m_player1->m_isUpsideDown};
            p1.push_back(newdata_p1);
            checkpoints_data newdata_p2 = {frame, self->m_player2->m_position.x, self->m_player2->m_position.y, 
                self->m_player2->getRotation(), self->m_player2->m_xAccel, self->m_player2->m_yAccel, self->m_player2->m_jumpAccel,
                    self->m_player2->m_playerSpeed, self->m_player2->m_isUpsideDown};
            p2.push_back(newdata_p2);
        }

        void fix(gd::PlayLayer* self) {
            if (!p1.empty()) {
                self->m_player1->m_position.x = p1.back().pos_x;
				self->m_player1->m_position.y = p1.back().pos_y;
				self->m_player1->setRotation(p1.back().rotation);
				self->m_player1->m_xAccel = p1.back().x_vel;
				self->m_player1->m_yAccel = p1.back().y_vel;
				self->m_player1->m_jumpAccel = p1.back().jump_vel;
				self->m_player1->m_playerSpeed = p1.back().player_speed;
				self->m_player1->m_isUpsideDown = p1.back().is_upsidedown;

				self->m_player2->m_position.x = p2.back().pos_x;
				self->m_player2->m_position.y = p2.back().pos_y;
				self->m_player2->setRotation(p2.back().rotation);
				self->m_player2->m_xAccel = p2.back().x_vel;
				self->m_player2->m_yAccel = p2.back().y_vel;
				self->m_player2->m_jumpAccel = p2.back().jump_vel;
				self->m_player2->m_playerSpeed = p2.back().player_speed;
				self->m_player2->m_isUpsideDown = p2.back().is_upsidedown;    
            }
        }
    }

    namespace frame {
        unsigned get_frame() {
            auto pl = gd::GameManager::sharedState()->getPlayLayer();
            if (pl) return static_cast<unsigned>(pl->m_time * info::fps) + frame_offset;
            return 0;
        }
        unsigned frame_offset = 0;
    }

    namespace inputs {
        bool p1 = false;
        bool p2 = false;
    }

    namespace info {
        int mode = state::disable;
        char replay_name[128] = "";
        float fps = 60.f;
        float speedhack = 1.f;
    }

    namespace fixes {
        bool practice_fix = true;
        bool accuracy_fix = true;
    }

    namespace advanced {
        bool ignore_input = true;
        bool speedhack_audio = false;
        extern bool dual_clicks = false;
        float respawn_time = 1.f;
    }

    namespace frameadvance {
        bool enabled = false;
        bool next_frame = false;
        bool holding = false;
        bool held = false;
    }

    namespace irecorder {
        Recorder recorder;
        bool recorder_c = false;
        char video_name[128];
    }

    namespace sequence {
        char replay_sq_name[128];
        vector<string> replays;
        int current_idx = 0;
        bool enable_sqp = false;
        bool random_sqp = false;
        bool first_sqp = true;
        void do_some_magic() {
            if (enable_sqp && info::mode == state::play) {
                if (!dashreplay::sequence::random_sqp) {
                    if (first_sqp)  {
                        current_idx = 0;
                        first_sqp = false;
                    }
                    else dashreplay::sequence::current_idx++;                    
                    if (dashreplay::sequence::replays.size() <= (size_t)dashreplay::sequence::current_idx) dashreplay::sequence::current_idx = 0;
                    strcpy_s(dashreplay::info::replay_name, dashreplay::sequence::replays[dashreplay::sequence::current_idx].c_str());
                    dashreplay::replay::load();
                }
                else {
                    dashreplay::sequence::current_idx = rand() % dashreplay::sequence::replays.size();
                    strcpy_s(dashreplay::info::replay_name, dashreplay::sequence::replays[dashreplay::sequence::current_idx].c_str());
                    dashreplay::replay::load();
                }            
            }
        }
    }

    namespace converter {
        int converterType = 0;

        void convert() {
            if (converterType == 0) {
                if (converterType == 0) {
                    std::ofstream out("DashReplay/converted.txt");
                    out << (int)dashreplay::info::fps << "\n";
                    for (int i = 0; i < (int)dashreplay::replay::p1.size(); i++) {
                        if (i == 0 || (dashreplay::replay::p1[i].down == dashreplay::replay::p1[i - 1].down && dashreplay::replay::p2[i].down == dashreplay::replay::p2[i - 1].down))
                            continue;
                        out << i << " " << dashreplay::replay::p1[i].down << " " << dashreplay::replay::p2[i].down << "\n";
                    }
                    out.close();	
                }
            }
        }
    }
}