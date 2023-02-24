#include "pch.h"
#include "recorder.h"

enum state {disable, record, play};

struct replay_data {
    unsigned frame;
    float x_pos;
    float y_pos;
    double y_vel;
    bool down;
};

struct checkpoints_data {
	unsigned frame;
    float pos_x;
    float pos_y;
    float rotation;
    double x_vel;
    double y_vel;
    double jump_vel;
    float player_speed;
    bool is_upsidedown;
};

namespace dashreplay {
    namespace replay {
        extern vector<replay_data> p1;
        extern vector<replay_data> p2;
        replay_data handle_recording(gd::PlayLayer* self, bool player);
        void handle_playing(gd::PlayLayer* self);
        void handle_reset(gd::PlayLayer* self);
        bool save();
        bool load();
    }   

    namespace practice_ex {
        extern vector<checkpoints_data> p1;
        extern vector<checkpoints_data> p2;
        void handle_checkpoint(gd::PlayLayer* self);
        void fix(gd::PlayLayer* self);
    }

    namespace frame {
        unsigned get_frame();
        extern unsigned frame_offset;
    }

    namespace inputs {
        extern bool p1;
        extern bool p2;
    }

    namespace info {
        extern int mode;
        extern char replay_name[128];
        extern float fps;
        extern float speedhack;
    }

    namespace advanced {
        extern bool ignore_input;
        extern bool speedhack_audio;
        extern bool dual_clicks;
        extern float respawn_time;
    }

    namespace fixes {
        extern bool practice_fix;
        extern bool accuracy_fix;
    }

    namespace frameadvance {
        extern bool enabled;
        extern bool next_frame;
        extern bool holding;
        extern bool held;
    }

    namespace irecorder {
        extern Recorder recorder;
        extern bool recorder_c;
        extern char video_name[128];
    }

    namespace sequence {
        extern char replay_sq_name[128];
        extern vector<string> replays;
        extern int current_idx;
        extern bool enable_sqp;
        extern bool random_sqp;
        extern bool first_sqp;
        void do_some_magic();
    }

    namespace converter {
        extern int converterType;
        void convert();
    }
}