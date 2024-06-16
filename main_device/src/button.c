#include "button.h"
#include "gpio.h"
#include "services/step.h"
#include "button_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

extern int plant_index;
extern int plant_quantity;
extern STEP current_step;

static void clean_up(void* arg) {
    BUTTON* btn = (BUTTON*)arg;
    GPIOUnexport(btn->pin);
    free(btn);
}

static void* button_polling(void* arg) {
    BUTTON* btn = (BUTTON*)arg;
    int interval = 1000000 / btn->polling_rate;
    int last_state = HIGH;
    int press_time = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(clean_up, btn);

    while (1) {
        char path[50];
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", btn->pin);
        int fd = open(path, O_RDONLY);
        if (fd == -1) {
            perror("Fail to open");
            continue;
        }

        char read_buff[3];
        if (read(fd, read_buff, sizeof(read_buff) - 1) == -1) {
            perror("Failed to read");
            close(fd);
            continue;
        }

        read_buff[2] = '\0';
        close(fd);

        int current_state = atoi(read_buff);

        if (current_state == LOW && last_state == HIGH) {
            btn->onPressDown(btn);
            press_time = 0;
        } else if (current_state == HIGH && last_state == LOW) {
            btn->onPressUp(btn);
            if (press_time >= 800000) {
                btn->onLongClick(btn);
            }
        }

        if (current_state == LOW) {
            press_time += interval;
        }

        last_state = current_state;
        usleep(interval);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

pthread_t* initButton(BUTTON* button) {
    GPIOUnexport(button->pin);
    usleep(100000);

    if (GPIOExport(button->pin) == -1) {
        fprintf(stderr, "failed to export pin %d\n", button->pin);
        return NULL;
    }

    usleep(100000);

    if (GPIODirection(button->pin, IN) == -1) {
        fprintf(stderr, "failed to set direction for pin %d\n", button->pin);
        return NULL;
    }

    pthread_t* thread = (pthread_t*)malloc(sizeof(pthread_t));
    BUTTON* btn = (BUTTON*)malloc(sizeof(BUTTON));
    *btn = *button;

    if (pthread_create(thread, NULL, button_polling, (void*)btn) != 0) {
        fprintf(stderr, "failed to create thread\n");
        free(thread);
        free(btn);
        return NULL;
    }

    return thread;
}

void dispose(pthread_t* thread) {
    if (pthread_cancel(*thread) != 0) {
        fprintf(stderr,"failed to cancel thread\n");
    }
    pthread_join(*thread, NULL);
    free(thread);
}
