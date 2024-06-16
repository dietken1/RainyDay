#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "gpio.h"
#include "button.h"
#include "config.h"
#include "services/button_service.h"
#include "services/lcd1602_service.h"
#include "services/communication_service.h"
#include "services/step.h"

// 설정 파일 경로
#define CONFIG_FILE "config.txt"


pthread_t lcd_thread;
pthread_t comm_thread;

// BUTTON 구조체 초기화
BUTTON *button1;
BUTTON *button2;

pthread_t *pt1;
pthread_t *pt2;

int main(void) {
    int pins[4];
    int polling_rates[2];

    if (read_config(CONFIG_FILE, pins, polling_rates) != 0) {
        return 1;
    }


    // 첫 번째 버튼 초기화
    button1 = (BUTTON *)malloc(sizeof(BUTTON));
    button1->pin = pins[0];
    button1->pout = pins[1];
    button1->polling_rate = polling_rates[0];
    button1->onLongClick = onLongClick;
    button1->onPressDown = onPressDown1;
    button1->onPressUp = onPressUp;

    // 첫 번째 버튼의 pull-up 설정
    if (GPIOExport(button1->pin) == -1) {
        printf("Failed to export GPIO pin for button 1\n");
        return 1;
    }
    usleep(100000); // GPIOExport 후 대기

    if (GPIOExport(button1->pout) == -1) {
        printf("Failed to export GPIO pout for button 1\n");
        return 1;
    }
    usleep(100000); // GPIOExport 후 대기

    if (GPIODirection(button1->pin, IN) == -1) {
        printf("Failed to set direction for GPIO pin for button 1\n");
        return 1;
    }
    usleep(100000); // GPIODirection 설정 후 대기

    if (GPIODirection(button1->pout, OUT) == -1) {
        printf("Failed to set direction for GPIO pout for button 1\n");
        return 1;
    }
    usleep(100000); // GPIODirection 설정 후 대기

    if (GPIOWrite(button1->pout, HIGH) != 0) {
        printf("Failed to set pull-up for button 1\n");
        return 1;
    }

    pt1 = initButton(button1);

    // 두 번째 버튼 초기화
    button2 = (BUTTON *)malloc(sizeof(BUTTON));
    button2->pin = pins[2];
    button2->pout = pins[3];
    button2->polling_rate = polling_rates[1];
    button2->onLongClick = onLongClick;
    button2->onPressDown = onPressDown2;
    button2->onPressUp = onPressUp;

    // 두 번째 버튼의 pull-up 설정
    if (GPIOExport(button2->pin) == -1) {
        printf("Failed to export GPIO pin for button 2\n");
        return 1;
    }
    usleep(100000); // GPIOExport 후 대기

    if (GPIOExport(button2->pout) == -1) {
        printf("Failed to export GPIO pout for button 2\n");
        return 1;
    }
    usleep(100000); // GPIOExport 후 대기

    if (GPIODirection(button2->pin, IN) == -1) {
        printf("Failed to set direction for GPIO pin for button 2\n");
        return 1;
    }
    usleep(100000); // GPIODirection 설정 후 대기

    if (GPIODirection(button2->pout, OUT) == -1) {
        printf("Failed to set direction for GPIO pout for button 2\n");
        return 1;
    }
    usleep(100000); // GPIODirection 설정 후 대기

    if (GPIOWrite(button2->pout, HIGH) != 0) {
        printf("Failed to set pull-up for button 2\n");
        return 1;
    }

    pt2 = initButton(button2);

    // LCD 출력 스레드 생성
    if (pthread_create(&lcd_thread, NULL, lcd_display_thread, NULL) != 0) {
        printf("Failed to create LCD display thread\n");
        return 1;
    }

    if (pt1 == NULL || pt2 == NULL) {
        printf("button init fail\n");
        return 1;
    } else {
        printf("Hello Buttons!\n");

        while (1) {
            sleep(1);
        }

        // 아래 코드는 더 이상 사용되지 않지만, 참고로 남겨둠
        // pthread_join(*pt1, NULL);
        // pthread_join(*pt2, NULL);
        // stop_lcd_thread(); // 종료 신호 전송
        // pthread_join(lcd_thread, NULL); // LCD 스레드 종료 대기
        // printf("buttons disposed. please enter to rerun\n");
        // char c[1000];
        // fgets(c, 1000, stdin);
    }
    // free(button1);
    // free(button2);
    return 0;
}
