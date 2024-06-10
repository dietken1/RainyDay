#ifndef LCD1602_H
#define LCD1602_H

void LCD_init(int addr, int bl);
void LCD_write(int x, int y, const char* str);
void LCD_clear();
void LCD_openLight();

#endif // LCD1602_H
