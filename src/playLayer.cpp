#include "playLayer.h"
#include "replayEngine.h"
#include "spambot.h"
#include "clicks.h"
#include "hacks.h"

namespace playLayer {
    bool __fastcall initHook(gd::PlayLayer* self, int edx, gd::GJGameLevel* level) {
        auto ret = playLayer::init(self, level);
        strcpy_s(dashreplay::info::replay_name, self->m_level->levelName.c_str());
        if (!dashreplay::irecorder::recorder.m_recording)
            strcpy_s(dashreplay::irecorder::video_name, string(self->m_level->levelName + ".mp4").c_str());
        dashreplay::frame::frame_offset = 0;
        dashreplay::practice_ex::p1.clear();
        dashreplay::practice_ex::p2.clear();
        return ret;
    }
    
    void __fastcall playLayer::updateHook(gd::PlayLayer* self, int edx, float deltaTime) {
        if (dashreplay::irecorder::recorder.m_recording)
            dashreplay::irecorder::recorder.handle_recording(self, deltaTime);
		playLayer::update(self, deltaTime);
        if (dashreplay::frame::get_frame() != 0) {
            if (dashreplay::info::mode == state::record) {
                dashreplay::replay::p1.push_back(dashreplay::replay::handle_recording(self, true));
                dashreplay::replay::p2.push_back(dashreplay::replay::handle_recording(self, false));
            }
            else if (dashreplay::info::mode == state::play) {                
                dashreplay::replay::handle_playing(self);
            }
        }

        if (spambot::enable) {
			if (spambot::next_frame()) {
				if (spambot::downed) {
					if (spambot::player1) { playLayer::releaseButtonHook(self, 0, 0, true);}
					if (spambot::player2) { playLayer::releaseButtonHook(self, 0, 0, false);} 
				}
				else {
					if (spambot::player1) { playLayer::pushButtonHook(self, 0, 0, true);}
					if (spambot::player2) { playLayer::pushButtonHook(self, 0, 0, false);} 
				}
			}
		}
    }

    void __fastcall playLayer::resetLevelHook(gd::PlayLayer* self) {
        dashreplay::sequence::do_some_magic();
        playLayer::resetLevel(self);
        dashreplay::replay::handle_reset(self);
    }

    void __fastcall playLayer::onQuitHook(gd::PlayLayer* self) {
        if (hacks::ignore_esc && !self->m_hasLevelCompleteMenu) {
            vector<string> msg = {"Wait...", "ESC Detected!", "Denied.", "Please stop!"};
            gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, msg[rand() % msg.size()].c_str())->show();
        }
        else {
            playLayer::onQuit(self);
            dashreplay::info::mode = state::disable;
            spambot::enable = false;
            dashreplay::sequence::first_sqp = true;
        }
    }

    void __fastcall playLayer::levelCompleteHook(gd::PlayLayer* self) {
        playLayer::levelComplete(self);
        if (dashreplay::info::mode = state::record) {
            dashreplay::info::mode = state::disable;
            spambot::enable = false;
        }
        
        if (self->m_isPracticeMode) {
            dashreplay::practice_ex::p1.clear();
            dashreplay::practice_ex::p2.clear();
        }
    }

    bool __fastcall playLayer::pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
        if (dashreplay::info::mode == state::play && dashreplay::advanced::ignore_input) return false;
        auto ret = playLayer::pushButton(self, state, player);	
        if (player) dashreplay::inputs::p1 = true;
        else dashreplay::inputs::p2 = true;	
        if (dashreplay::advanced::dual_clicks) {
			dashreplay::inputs::p1 = true;
			dashreplay::inputs::p2 = true;
			playLayer::pushButton(self, state, !player);
		}
        return ret;	
    }

    bool __fastcall playLayer::releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
        if (dashreplay::info::mode == state::play && dashreplay::advanced::ignore_input) return false;
        auto ret = playLayer::releaseButton(self, state, player);
        if (player) dashreplay::inputs::p1 = false;
        else dashreplay::inputs::p2 = false;	
        if (dashreplay::advanced::dual_clicks) {
			dashreplay::inputs::p1 = false;
			dashreplay::inputs::p2 = false;
			playLayer::releaseButton(self, state, !player);
		}
        return ret;
    }

    int __fastcall playLayer::createCheckpointHook(gd::PlayLayer* self) {
        auto ret = playLayer::createCheckpoint(self);
        if (dashreplay::info::mode == state::record) dashreplay::practice_ex::handle_checkpoint(self);
        return ret; 
    }

    int __fastcall playLayer::removeCheckpointHook(gd::PlayLayer* self) {
        auto ret = playLayer::removeCheckpoint(self);
        if (dashreplay::info::mode == state::record) {
            dashreplay::practice_ex::p1.pop_back();
            dashreplay::practice_ex::p2.pop_back();
        }
        return ret;
    }

    void __fastcall playLayer::togglePracticeHook(void* self, int edx, bool practice) {
        togglePractice(self, practice);
        dashreplay::practice_ex::p1.clear();
        dashreplay::practice_ex::p2.clear();
        if (!practice) dashreplay::frame::frame_offset = 0;
    }

    bool __fastcall playLayer::onEditorButtonClickHook(gd::LevelEditorLayer* self, int edx, gd::GJGameLevel* level) {
        auto ret = playLayer::onEditorButtonClick(self, level);
        dashreplay::sequence::first_sqp = true;
        dashreplay::info::mode = state::disable;
        spambot::enable = false;
        return ret;
    }

    void mem_init() {
        MH_CreateHook((PVOID)(gd::base + 0x01FB780), playLayer::initHook, (LPVOID*)&playLayer::init);
        MH_CreateHook((PVOID)(gd::base + 0x2029C0), playLayer::updateHook, (LPVOID*)&playLayer::update);
        MH_CreateHook((PVOID)(gd::base + 0x20BF00), playLayer::resetLevelHook, (LPVOID*)&playLayer::resetLevel);
        MH_CreateHook((PVOID)(gd::base + 0x20D810), playLayer::onQuitHook, (LPVOID*)&playLayer::onQuit);
        MH_CreateHook((PVOID)(gd::base + 0x1FD3D0), playLayer::levelCompleteHook, (LPVOID*)&playLayer::levelComplete);
        MH_CreateHook((PVOID)(gd::base + 0x111500), playLayer::pushButtonHook, (LPVOID*)&playLayer::pushButton);
	    MH_CreateHook((PVOID)(gd::base + 0x111660), playLayer::releaseButtonHook, (LPVOID*)&playLayer::releaseButton);
        MH_CreateHook((PVOID)(gd::base + 0x20B050),	playLayer::createCheckpointHook, (LPVOID*)&playLayer::createCheckpoint);
	    MH_CreateHook((PVOID)(gd::base + 0x20B830), playLayer::removeCheckpointHook, (LPVOID*)&playLayer::removeCheckpoint);
        MH_CreateHook((PVOID)(gd::base + 0x20D0D0), playLayer::togglePracticeHook, (LPVOID*)&togglePractice);
        MH_CreateHook((PVOID)(gd::base + 0x15EE00), playLayer::onEditorButtonClickHook, (LPVOID*)&playLayer::onEditorButtonClick);
    } 
}