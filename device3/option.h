#include <pthread.h>

typedef struct option {
    char type[9];
    char devName[50];
    int polling_rate;
    int value;
} OPTION;

pthread_t* initGetLight(OPTION* option);
pthread_t* initLED(OPTION* option);

