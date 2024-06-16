#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "LCD1602.h"


int lcd_addr; // LCD의 I2C 주소
int backlight_enable; // 백라이트 상태
int i2c_file; // I2C 파일 디스크립터


void write_word(int data) {
    int temp = data;

    if (backlight_enable == 1) {
        temp |= 0x08; // 백라이트 ON
    } else {
        temp &= 0xF7; // 백라이트 OFF
    }
    i2c_smbus_write_byte(i2c_file, temp); 
}

// 명령어를 LCD에 보냄
void send_command(int comm) {
    int buf = comm & 0xF0; // 상위 4비트 추출 (2진수: 11110000 와 AND 연산)
    buf |= 0x04; // E 비트 설정 (RS = 0, RW = 0, E = 1) (2진수: 00000100)
    write_word(buf); // 상위 4비트 전송
    usleep(2000); 
    buf &= 0xFB; // E 비트 클리어 (E = 0) (2진수: 11111011 와 AND 연산)
    write_word(buf); // 상위 4비트 전송 완료

    buf = (comm & 0x0F) << 4; // 하위 4비트 추출 후 이동 (2진수: 00001111 와 AND 연산 후 왼쪽으로 4비트 이동)
    buf |= 0x04; // 명령어 설정 (2진수: 00000100)
    write_word(buf); 
    usleep(2000); 
    buf &= 0xFB; // E 비트 클리어 (E = 0) (2진수: 11111011 와 AND 연산)
    write_word(buf);
}

// 데이터를 LCD에 보냄
void send_data(int data) {
    int buf = data & 0xF0; // 상위 4비트 추출
    buf |= 0x05; // e비트 및 rs 비트까지 같이 넣어줌
    write_word(buf); 
    usleep(2000); 
    buf &= 0xFB; // E 비트 클리어 
    write_word(buf); 

    buf = (data & 0x0F) << 4; // 하위 4비트 추출 후 이동
    buf |= 0x05;
    write_word(buf); 
    usleep(2000); 
    buf &= 0xFB; 
    write_word(buf); 
}

// // LCD 초기화 함수
// lcd가 읽을때 4비트 모드로 읽을거임
// ic2버스는 8비트짜리인데 상위 4비트가 명령어 하위4비트가 제어비트로 해석함
void LCD_init(int addr, int bl) {
    lcd_addr = addr; // LCD I2C 주소 설정
    backlight_enable = bl; // 백라이트 설정

    char *filename = (char*)"/dev/i2c-1"; // I2C 디바이스 파일 경로
    if ((i2c_file = open(filename, O_RDWR)) < 0) { // I2C 디바이스 파일 열기
        printf("Failed to open the i2c bus\n");
        exit(1); 
    }

    if (ioctl(i2c_file, I2C_SLAVE, lcd_addr) < 0) { // I2C 슬레이브 설정
        printf("Failed to acquire bus access and/or talk to slave\n"); 
        exit(1); 
    }

    // LCD 초기화 명령 전송
    send_command(0x33); // 8비트 모드로 초기화 (이진수: 0011 0011)
    usleep(5000);
    send_command(0x32); // 4비트 모드로 전환 (이진수: 0011 0010)
    usleep(5000);                                                           
    send_command(0x28); // 4비트 모드, 2라인, 5x8 도트 설정 (이진수: 0010 1000)
    usleep(5000);
    send_command(0x0C); // 디스플레이 ON, 커서 OFF (이진수: 0000 1100)
    usleep(5000); 
    send_command(0x01); // 디스플레이 클리어 (이진수: 0000 0001)
    i2c_smbus_write_byte(i2c_file, 0x08); // 백라이트 ON (이진수: 0000 1000)
}

// LCD 화면을 클리어하는 함수
void LCD_clear() {
    send_command(0x01); // 디스플레이 클리어 명령
}

// LCD 백라이트를 켜는 함수
void LCD_openLight() {
    i2c_smbus_write_byte(i2c_file, 0x08); // 백라이트 ON
    close(i2c_file); 
}

// 특정 위치에 문자열을 쓰는 함수
void LCD_write(int x, int y, const char* str) {
    // 좌표 값 범위 체크
    if (x < 0) x = 0;
    if (x > 15) x = 15;
    if (y < 0) y = 0;
    if (y > 1) y = 1;

    // DDRAM 주소 설정
    int addr = 0x80 + 0x40 * y + x; // 기본위치가 0x80이고 y가 새로,x가 가로임
    send_command(addr); 


    for (int i = 0; i < strlen(str); i++) {
        send_data(str[i]); 
    }
}
