#ifndef BUTTON_SERVICE_H
#define BUTTON_SERVICE_H

#include "button.h"

void* step1onPressDown1(BUTTON* btn);
void* step1onPressDown2(BUTTON* btn);
void* step2onPressDown1(BUTTON* btn);
void* step2onPressDown2(BUTTON* btn);

void* onLongClick(BUTTON* btn);
void* onPressDown1(BUTTON* btn);
void* onPressDown2(BUTTON* btn);
void* onPressUp(BUTTON* btn);

#endif // BUTTON_SERVICE_H
