#ifndef STEP_H
#define STEP_H

typedef enum {
    STEP1,
    STEP2
} STEP;

extern STEP current_step;

const char* stepToString(STEP step);

#endif // STEP_H
