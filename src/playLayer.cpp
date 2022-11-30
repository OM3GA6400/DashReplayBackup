#include "playLayer.h"
#include "console.h"

int playLayer::mode = 0;
int playLayer::frame = 0;

bool playLayer::down_p1 = false;
bool playLayer::down_p2 = false;

bool playLayer::ignore_input = false;
bool playLayer::dual_clicks = false;

float playLayer::fpsvalue = 60.f;
float playLayer::speedvalue = 1.f;

vector<replaydata> playLayer::replay_p1;
vector<replaydata> playLayer::replay_p2;

vector<checkpointsdata> playLayer::checkpoints_p1;
vector<checkpointsdata> playLayer::checkpoints_p2;

namespace playLayer {
	bool playLayer::saveReplay(string s) {
		if (replay_p1.empty()) return false;
		fstream file(s, std::ios::out);
        if (file.is_open()) {
            file << "DashReplay Engine v3.1.0 //frame, pos_x, pos_y, rotation, y_vel, isDown" << "\n";
			file << "Framerate: " << FPSMultiplier::g_target_fps << "\n";
			file << "1P Replay Size: " << replay_p1.size() << "\n";
			for (int i = 0; i < (int)replay_p1.size(); i++) {
				file << to_string(replay_p1[i].frame) << " " << to_string(replay_p1[i].pos_x) << " " <<
					to_string(replay_p1[i].pos_y) << " " << to_string(replay_p1[i].rotation) << " " <<
						to_string(replay_p1[i].y_vel) << " " << uselessShit::boolToString(replay_p1[i].down)  << "\n";
			}
			file << "2P Replay Size: " << replay_p2.size() << "\n";
			for (int i = 0; i < (int)replay_p2.size(); i++) {
				file << replay_p2[i].frame << " " << to_string(replay_p2[i].pos_x) << " " <<
					to_string(replay_p2[i].pos_y) << " " << to_string(replay_p2[i].rotation) << " " <<
						to_string(replay_p2[i].y_vel) << " " << uselessShit::boolToString(replay_p2[i].down) << "\n";
			}
        }
        return true;
	}

	bool playLayer::loadReplay(string s) {
		string line;
		string splitword;
		int replaysize;
		vector<string> splitwords;

		fstream file(s, std::ios::in);
        if (!file.is_open())
            return false;
		getline(file, line);
		if (line.substr(0, 24) != "DashReplay Engine v3.1.0") {
			if (MessageBoxA(0, "Replay was recorded with old version of DashReplay. In this case, the replay may be broken. Continue?", "DashReplay", MB_YESNO) == IDNO) 
				return false;
		}
		getline(file, line);
		if (line.substr(0, 11) == "Framerate: ") {
			line.erase(0, 11);
			FPSMultiplier::g_target_fps = stof(line);
		}
		getline(file, line);
		if (line.substr(0, 16) == "1P Replay Size: ") {
			line.erase(0, 16);
			replaysize = stoi(line);
		}
		for (int i = 0; i < replaysize; i++) {
			getline(file, line);
			istringstream splitstr(line);
			splitwords.clear();
            while (getline(splitstr, splitword, ' ')) {
                splitwords.push_back(splitword);
            }
			replaydata newdata = {stoi(splitwords[0]), stof(splitwords[1]), stof(splitwords[2]), stof(splitwords[3]),
				stof(splitwords[4]), uselessShit::stringToBool(splitwords[5])};
			replay_p1.push_back(newdata);
		}
		getline(file, line);
		if (line.substr(0, 16) == "2P Replay Size: ") {
			line.erase(0, 16);
			replaysize = stoi(line);
		}
		for (int i = 0; i < replaysize; i++) {
			getline(file, line);
			istringstream splitstr(line);
			splitwords.clear();
            while (getline(splitstr, splitword, ' ')) {
                splitwords.push_back(splitword);
            }
			replaydata newdata = {stoi(splitwords[0]), stof(splitwords[1]), stof(splitwords[2]), stof(splitwords[3]),
				stof(splitwords[4]), uselessShit::stringToBool(splitwords[5])};
			replay_p2.push_back(newdata);
		}
		return true;
	}

	void playLayer::clearMacro() {
		replay_p1.clear();
		replay_p2.clear();
        checkpoints_p1.clear();
        checkpoints_p2.clear();
	}

    bool __fastcall playLayer::initHook(gd::PlayLayer* self, int edx, gd::GJGameLevel* level) {
		bool ret = playLayer::init(self, level);

		return ret;
	}

    void __fastcall playLayer::updateHook(gd::PlayLayer* self, int edx, float deltaTime) {
		playLayer::update(self, deltaTime);
		if (self->m_pPlayer1->m_position.x != 0 && !self->m_isDead) frame++;
		if (mode == 1) {
			if (frame) {
				replaydata newdata_p1 = {frame, self->m_pPlayer1->m_position.x, self->m_pPlayer1->m_position.y, self->m_pPlayer1->getRotation(),
					(float)self->m_pPlayer1->m_yAccel, down_p1};
				replay_p1.push_back(newdata_p1);

				replaydata newdata_p2 = {frame, self->m_pPlayer2->m_position.x, self->m_pPlayer2->m_position.y, self->m_pPlayer2->getRotation(),
					(float)self->m_pPlayer2->m_yAccel, down_p2};
				replay_p2.push_back(newdata_p2);
			}
		}
		else if (mode == 2 && !self->m_isDead) {
			if (frame) {
				if ((int)replay_p1.size() > frame) {
					if (replay_p1[frame-1].pos_x != -1) self->m_pPlayer1->m_position.x = replay_p1[frame-1].pos_x;
					if (replay_p1[frame-1].pos_y != -1) self->m_pPlayer1->m_position.y = replay_p1[frame-1].pos_y;
					//if (replay_p1[frame-1].rotation != -1) self->m_pPlayer1->setRotation(replay_p1[frame-1].rotation);
					if (replay_p1[frame-1].y_vel != -1) self->m_pPlayer1->m_yAccel = replay_p1[frame-1].y_vel;
					if (replay_p1[frame].down != -1) {
						if (replay_p1[frame].down && !down_p1) {
							pushButton(self, 0, true);
							down_p1 = true;
						}
						else if (!replay_p1[frame].down && down_p1) {
							releaseButton(self, 0, true);
							down_p1 = false;
						}
					}
				}

				if ((int)replay_p2.size() > frame) {
					if (replay_p2[frame-1].pos_x != -1) self->m_pPlayer2->m_position.x = replay_p2[frame-1].pos_x;
					if (replay_p2[frame-1].pos_y != -1) self->m_pPlayer2->m_position.y = replay_p2[frame-1].pos_y;
					//if (replay_p2[frame-1].rotation != -1) self->m_pPlayer2->setRotation(replay_p2[frame-1].rotation);
					if (replay_p2[frame-1].y_vel != -1) self->m_pPlayer2->m_yAccel = replay_p2[frame-1].y_vel;
					if (replay_p1[frame].down != -1) {
						if (replay_p2[frame].down && !down_p2) {
							pushButton(self, 0, false);
							down_p2 = true;
						}
						else if (!replay_p2[frame].down && down_p2) {
							releaseButton(self, 0, false);
							down_p2 = false;
						}
					}
				}
			}
		}

		if (spambot::enable) {
			if (spambot::NextFrame()) {
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
		playLayer::resetLevel(self);
		if (self->m_isPracticeMode) {
			checkpointsdata fuckning_data = {0};
			if (checkpoints_p1.empty()) checkpoints_p1.push_back(fuckning_data);
			if (checkpoints_p2.empty()) checkpoints_p2.push_back(fuckning_data);

			while ((int)replay_p1.size() > checkpoints_p1.back().frame) replay_p1.pop_back();
			while ((int)replay_p2.size() > checkpoints_p2.back().frame) replay_p2.pop_back();

			if (checkpoints_p1.back().frame != 0) {
				frame = checkpoints_p1.back().frame;
				self->m_pPlayer1->m_position.x = checkpoints_p1.back().pos_x;
				self->m_pPlayer1->m_position.y = checkpoints_p1.back().pos_y;
				self->m_pPlayer1->setRotation(checkpoints_p1.back().rotation);
				self->m_pPlayer1->m_xAccel = checkpoints_p1.back().x_vel;
				self->m_pPlayer1->m_yAccel = checkpoints_p1.back().y_vel;
				self->m_pPlayer1->m_jumpAccel = checkpoints_p1.back().jump_vel;
				self->m_pPlayer1->m_playerSpeed = checkpoints_p1.back().player_speed;
				self->m_pPlayer1->m_isUpsideDown = checkpoints_p1.back().is_upsidedown;

				self->m_pPlayer2->m_position.x = checkpoints_p2.back().pos_x;
				self->m_pPlayer2->m_position.y = checkpoints_p2.back().pos_y;
				self->m_pPlayer2->setRotation(checkpoints_p2.back().rotation);
				self->m_pPlayer2->m_xAccel = checkpoints_p2.back().x_vel;
				self->m_pPlayer2->m_yAccel = checkpoints_p2.back().y_vel;
				self->m_pPlayer2->m_jumpAccel = checkpoints_p2.back().jump_vel;
				self->m_pPlayer2->m_playerSpeed = checkpoints_p2.back().player_speed;
				self->m_pPlayer2->m_isUpsideDown = checkpoints_p2.back().is_upsidedown;
			}
			else frame = 0;


		}
		else {
			if (checkpoints_p1.empty()) frame = 0;
		}

    }

    void __fastcall playLayer::onQuitHook(gd::PlayLayer* self) {
		playLayer::onQuit(self);	
		frame = 0;	
		mode = 0;
		FPSMultiplier::frame_advance = false;
		FPSMultiplier::nextframe = false;
	}

    void __fastcall playLayer::levelCompleteHook(gd::PlayLayer* self) {
        playLayer::levelComplete(self);
		if (mode == 1) {
			checkpoints_p1.clear();
			checkpoints_p2.clear();
			mode = 0;
		}
	}

	bool __fastcall playLayer::pushButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
		auto ret = playLayer::pushButton(self, state, player);
		if (player) down_p1 = true;
		if (!player) down_p2 = true;
		if (dual_clicks) {
			down_p1 = true;
			down_p2 = true;
			playLayer::pushButton(self, state, !player);
		}
		return ret;
	}

	bool __fastcall playLayer::releaseButtonHook(gd::PlayLayer* self, uintptr_t, int state, bool player) {
		auto ret = playLayer::releaseButton(self, state, player);
		if (player) down_p1 = false;
		if (!player) down_p2 = false;
		if (dual_clicks) {
			down_p1 = false;
			down_p2 = false;
			playLayer::releaseButton(self, state, !player);
		}
		return ret;
	}

    int __fastcall playLayer::createCheckpointHook(gd::PlayLayer* self) {
        auto ret = playLayer::createCheckpoint(self);
		checkpointsdata newdata_p1 = {frame, self->m_pPlayer1->m_position.x, self->m_pPlayer1->m_position.y, 
			self->m_pPlayer1->getRotation(), (float)self->m_pPlayer1->m_xAccel, (float)self->m_pPlayer1->m_yAccel, (float)self->m_pPlayer1->m_jumpAccel,
				self->m_pPlayer1->m_playerSpeed, self->m_pPlayer1->m_isUpsideDown};
		checkpoints_p1.push_back(newdata_p1);
		checkpointsdata newdata_p2 = {frame, self->m_pPlayer2->m_position.x, self->m_pPlayer2->m_position.y, 
			self->m_pPlayer2->getRotation(), (float)self->m_pPlayer2->m_xAccel, (float)self->m_pPlayer2->m_yAccel, (float)self->m_pPlayer2->m_jumpAccel,
				self->m_pPlayer2->m_playerSpeed, self->m_pPlayer2->m_isUpsideDown};
		checkpoints_p2.push_back(newdata_p2);
		return ret;
	}

	int __fastcall playLayer::removeCheckpointHook(gd::PlayLayer* self) {
        auto ret = playLayer::removeCheckpoint(self);
		if (!checkpoints_p1.empty()) checkpoints_p1.pop_back();
		if (!checkpoints_p2.empty()) checkpoints_p2.pop_back();
		return ret;
	}

    void playLayer::mem_init() {
        size_t base = (size_t)GetModuleHandle(0);

		MH_CreateHook(
			(PVOID)(base + 0x01FB780),
			playLayer::initHook,
			(LPVOID*)&playLayer::init
		);

        MH_CreateHook(
			(PVOID)(base + 0x2029C0),
			playLayer::updateHook,
			(LPVOID*)&playLayer::update
		);

        MH_CreateHook(
			(PVOID)(base + 0x20BF00),
			playLayer::resetLevelHook,
			(LPVOID*)&playLayer::resetLevel
		);

        MH_CreateHook(
			(PVOID)(base + 0x20D810),
			playLayer::onQuitHook,
			(LPVOID*)&playLayer::onQuit
		);

        MH_CreateHook(
			(PVOID)(base + 0x1FD3D0),
			playLayer::levelCompleteHook,
			(LPVOID*)&playLayer::levelComplete
		);

		MH_CreateHook(
			(PVOID)(base + 0x111500),
			playLayer::pushButtonHook,
			(LPVOID*)&playLayer::pushButton
		);

		MH_CreateHook(
			(PVOID)(base + 0x111660),
			playLayer::releaseButtonHook,
			(LPVOID*)&playLayer::releaseButton
		);

        MH_CreateHook(
			(PVOID)(base + 0x20B050),
			playLayer::createCheckpointHook,
			(LPVOID*)&playLayer::createCheckpoint
		);

		MH_CreateHook(
			(PVOID)(base + 0x20B830),
			playLayer::removeCheckpointHook,
			(LPVOID*)&playLayer::removeCheckpoint
		);
    }


}

