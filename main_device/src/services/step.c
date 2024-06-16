#include "services/step.h"

STEP current_step = STEP1;

const char* stepToString(STEP step) {
    switch(step) {
        case STEP1: return "STEP1";
        case STEP2: return "STEP2";
        default: return "UNKNOWN";
    }
}
