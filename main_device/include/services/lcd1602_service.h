#ifndef LCD1602_SERVICE_H
#define LCD1602_SERVICE_H

void update_lcd_message(const char *top_message, const char *bottom_message);
void* lcd_display_thread(void* arg);

#endif // LCD1602_SERVICE_H
