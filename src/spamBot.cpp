#include "spamBot.h"

extern bool spambot::enable = false;

extern bool spambot::player1 = true;
extern bool spambot::player2 = true;

int temppush = 0;
int temprelease = 0;

extern int spambot::push = 5;
extern int spambot::release = 5;

extern bool spambot::downed = false;

namespace spambot {
    bool spambot::NextFrame() {
        if (!downed) {
            if (temppush >= push-1) {
                temppush = 0;
                downed = !downed;
                return true;
            }
            else {
                temppush++;
                return false;
            }
        }
        else {
            if (temprelease >= release -1) {
                temprelease = 0;
                downed = !downed;
                return true;
            }
            else {
                temprelease++;
                return false;
            }
        }
    }
}