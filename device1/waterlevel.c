#include <fcntl.h>              //파일 제어를 위한 상수 및 함수 사용 목적
#include <linux/spi/spidev.h>   //SPI장치 제어 목적
#include <linux/types.h>
#include <stdint.h>             //표준 정수형 사용 목적
#include <stdio.h>              //표준 입출력 함수 사용 목적
#include <stdlib.h>             //표준 라이브러리
#include <sys/ioctl.h>          //입출력 제어함수 사용 목적
#include <unistd.h>             //표준 심볼 상수 및 자료형 사용 목적
#include <string.h>             //문자열 처리 라이브러리
#include <sys/socket.h>         //소켓 통신 라이브러리
#include <arpa/inet.h>          //네트워크 소켓을 위한 라이브러리
#include <json-c/json.h>        //JSON 라이브러리

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define SPI_PATH "/dev/spidev0.0"  //SPI장치 파일 경로 정의
#define ADC_CHANNEL 0              //ADC 채널 0
#define IP "192.168.75.4"          //서버 IP주소
#define PORT 12345                 //서버 포트번호

static uint8_t MODE = 0;           //SPI 모드
static uint8_t BITS = 8;           //SPI 비트 수
static uint32_t CLOCK = 1000000;   //SPI 속도
static uint16_t DELAY = 5;         //지연시간

//SPI를 통해 ADC값을 읽는 함수
int readADC(int fd, int channel) {
    uint8_t buffer[3];                   //ADC값을 읽기 위해서는 3바이트 버퍼가 필요
    buffer[0] = 1;                       //시작 비트
    buffer[1] = (8 + channel) << 4;      //채널 설정 (8은 ADC의 단일 모드 시작 비트)
    buffer[2] = 0;                       //버퍼를 채우기 위한 의미없는 데이터

    struct spi_ioc_transfer tr = {       //SPI전송 설정 구조체를 초기화
        .tx_buf = (unsigned long)buffer, //전송 버퍼
        .rx_buf = (unsigned long)buffer, //수신 버퍼
        .len = ARRAY_SIZE(buffer),       //데이터 길이
        .delay_usecs = DELAY,            //지연 시간
        .speed_hz = CLOCK,               //SPI속도
        .bits_per_word = BITS,           //단어당 비트 수
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) { //SPI 메시지 전송
        perror("IO Error");
        abort();
    }

    int adcValue = ((buffer[1] & 3) << 8) + buffer[2]; //수신된 데이터를 ADC값으로 변환
    return adcValue;                                   //ADC 값을 반환
}

//ADC 값을 서버에 JSON 형식으로 전송하는 함수
void sendValue(int adcValue) {
    int sck;                        //소켓 파일 디스크립터
    struct sockaddr_in serv_addr;   //서버 주소 구조체
    char response[256];             //서버로부터 응답을 받을 버퍼

    sck = socket(PF_INET, SOCK_STREAM, 0); //소켓 생성
    if (sck == -1) {                       //에러 처리
        perror("Socket Error");
        abort();
    }

    memset(&serv_addr, 0, sizeof(serv_addr));  //서버 주소 구조체 초기화
    serv_addr.sin_family = AF_INET;            //주소 체계 설정
    serv_addr.sin_addr.s_addr = inet_addr(IP); //서버 IP 주소 설정
    serv_addr.sin_port = htons(PORT);          //서버 포트 번호 설정

    if (connect(sck, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) { //서버에 연결
        perror("Connection Error");
        close(sck);
        abort();
    }

    //JSON형식의 Request 객체 생성
    struct json_object* request_json = json_object_new_object();                                //객체 생성
    json_object_object_add(request_json, "type", json_object_new_string("sensor"));             //요청에 "type": "sensor" 추가
    json_object_object_add(request_json, "sensor_type", json_object_new_string("water_level")); //요청에 "sensor_type": "water_level" 추가
    json_object_object_add(request_json, "value", json_object_new_int(adcValue));               //요청에 "value": adcValue 추가
    const char* request_str = json_object_to_json_string(request_json);                         //JSON 객체를 문자열로 변환해주는 함수

    //Request
    if (write(sck, request_str, strlen(request_str)) == -1) {  //서버에 Request전송
        perror("Write Error");
        json_object_put(request_json);  //JSON 객체를 해제해주는 함수
        close(sck);
        abort();
    }
    json_object_put(request_json);

    //Response
    int str_len = read(sck, response, sizeof(response) - 1); //서버로부터 받은 Response를 읽음
    if (str_len == -1) {                                     //오류 처리
        perror("Read Error");
        close(sck);
        abort();
    }
    response[str_len] = '\0'; //응답의 끝에 null 문자를 추가하여 문자열로 만듦

    //Response->Json 파싱
    struct json_object* parsed_json; //파싱된 JSON 객체
    struct json_object* status;      //JSON 객체 내의 status 필드

    parsed_json = json_tokener_parse(response); //응답을 파싱하여 JSON 객체로 변환
    if (parsed_json == NULL) {
        fprintf(stderr, "Parsing Error\n");
        close(sck);
        abort();
    }

    if (!json_object_object_get_ex(parsed_json, "status", &status)) { //변환한 객체에서 status필드 추출
        fprintf(stderr, "Field Error\n");
        json_object_put(parsed_json);
        close(sck);
        abort();
    }

    const char* status_str = json_object_get_string(status); //status필드에서 문자열 값 추출

    if (strcmp(status_str, "ok") == 0) {
        printf("Server Response: OK\n");    //통신 성공
    }
    else {
        printf("Server Response: ERROR\n"); //통신 실패
    }

    json_object_put(parsed_json);           //JSON 객체 해제
    close(sck);
}

//SPI 장치 설정 함수
static int prepare(int fd) {
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {          //모드설정
        perror("Can't set MODE");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) { //비트수설정
        perror("Can't set number of BITS");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) { //쓰기속도설정
        perror("Can't set write CLOCK");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) { //읽기속도설정
        perror("Can't set read CLOCK");
        return -1;
    }
    
    return 0;
}

//메인 함수
int main() {
    int fd = open(SPI_PATH, O_RDWR); //SPI 디바이스 파일 열기
    if (fd < 0) {
        perror("Open Error");
        return -1;
    }

    if (prepare(fd) == -1) {
        perror("Device prepare error");
        return -1;
    }

    //주기적으로 센서값을 읽어서 서버에 전송
    while (1) {
        int adcValue = readADC(fd, ADC_CHANNEL); //ADC 값 읽기
        if (adcValue >= 0) {
            printf("Value: %d\n", adcValue);     //측정값 출력
            sendValue(adcValue);                 //서버에 측정값 전송
        }
        usleep(1000000);                         //1초 대기
    }
    close(fd);  //SPI 디바이스 파일 닫기
    return 0;   //프로그램 종료
}
