#ifndef BUTTON_H
#define BUTTON_H

#include <pthread.h>

typedef struct button BUTTON;

typedef void* (*ButtonCallback)(BUTTON*);

struct button {
    int pin;
    int pout;
    int polling_rate;
    ButtonCallback onLongClick;
    ButtonCallback onPressDown;
    ButtonCallback onPressUp;
};

pthread_t* initButton(BUTTON* button);
void dispose(pthread_t* thread);

#endif // BUTTON_H
