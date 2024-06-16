#include "lcd1602_service.h"
#include "LCD1602.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t lcd_mutex = PTHREAD_MUTEX_INITIALIZER;

char lcd_message_top[32] = "STEP1";
char lcd_message_bottom[32] = "select plant";

// 변수를 업데이트만 해주는 함수임
void update_lcd_message(const char *top_message, const char *bottom_message) {
    // 현재는 버튼 스레드에서 호출함
    pthread_mutex_lock(&lcd_mutex); // 데이터 일관성과 동기화를 위해 lock

    strncpy(lcd_message_top, top_message, sizeof(lcd_message_top) - 1);
    lcd_message_top[sizeof(lcd_message_top) - 1] = '\0';
    strncpy(lcd_message_bottom, bottom_message, sizeof(lcd_message_bottom) - 1);
    lcd_message_bottom[sizeof(lcd_message_bottom) - 1] = '\0';

    pthread_mutex_unlock(&lcd_mutex);
}

void* lcd_display_thread(void* arg) {
    (void)arg; 
    
    LCD_init(0x27, 1); // I2C 주소에 맞게 설정
    while (1) {
        pthread_mutex_lock(&lcd_mutex); // 마찬가지로 lock

        LCD_clear();
        LCD_write(0, 0, lcd_message_top);
        LCD_write(0, 1, lcd_message_bottom);
        pthread_mutex_unlock(&lcd_mutex);

        sleep(2);
    }
    return NULL;
}
