#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "option.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cjson/cJSON.h>

pthread_t *getLightThread;
pthread_t *LEDThread;
char buffer[1024] = {0};

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

// 메시지 전송 및 수신
cJSON *sendMsg(OPTION *option, int sock, char *message)
{
    cJSON *json = cJSON_CreateObject();
    cJSON *receiveJSON;
    int str_len = 0;

    cJSON_AddStringToObject(json, "type", option->type);
    if(strcmp(option->type, "sensor") == 0)
    {
        cJSON_AddStringToObject(json, "sensor_type", option->devName);
        cJSON_AddNumberToObject(json, "value", option->value);
    }
    else
    {
        cJSON_AddStringToObject(json, "actuator_type", option->devName);
    }

    message = cJSON_Print(json); // json 객체 문자열로 변환
    write(sock, message, strlen(message));
    str_len = read(sock, buffer, 1024);
    if (str_len == -1)
        error_handling("read() error");
    free(json);
    receiveJSON = cJSON_Parse(buffer);
    return (receiveJSON);
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr; // 서버 주소 구조체
    char *message = 0;
    int signal = 1;
    char *msg;

    if (argc != 5)
    {
        printf("Usage : %s <IP> <port> <send_second> <polling_rate>\n", argv[0]);
        exit(1);
    }

    // 쓰레드를 위한 구조체
    OPTION *getLightOption = (OPTION *)calloc(1, sizeof(OPTION));
    strcpy(getLightOption->type, "sensor");
    strcpy(getLightOption->devName, "light_intensity");
    getLightOption->polling_rate = atoi(argv[4]);
    getLightOption->value = 0;

    OPTION *LEDOption = (OPTION *)calloc(1, sizeof(OPTION));
    strcpy(LEDOption->type, "actuator");
    strcpy(LEDOption->devName, "led");
    LEDOption->polling_rate = atoi(argv[4]);
    LEDOption->value = 0;

    // 쓰레드 호출
    if ((getLightThread = initGetLight(getLightOption)) == NULL)
    { // initLED
        printf("getLight init fail\n");
        return 1;
    }

    if ((LEDThread = initLED(LEDOption)) == NULL)
    {
        printf("LED init fail\n");
        return 1;
    }

    printf("Hello GetLight & LED!\n");

    while (1)
    {
        cJSON *receiveJSON;

        sock = socket(PF_INET, SOCK_STREAM, 0); // 소켓 생성
        if (sock == -1)
            error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr)); // 서버 주소 구조체 초기화
        serv_addr.sin_family = AF_INET; // 주소 체계 설정 (IPv4)
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]); // IP 주소 설정
        serv_addr.sin_port = htons(atoi(argv[2])); // 포트 번호 설정

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) // 서버 연결
            error_handling("connect() error");
        if(signal > 0) // 조도 값 전송
        {
            receiveJSON = sendMsg(getLightOption, sock, message);
            cJSON *status = cJSON_GetObjectItem(receiveJSON, "status");
            msg = cJSON_Print(status);
            signal *= -1;
        }
        else // 조도 값에 기반하여 led 제어.
        {
            receiveJSON = sendMsg(LEDOption, sock, message);
            cJSON *action = cJSON_GetObjectItem(receiveJSON, "led_value");
            msg = cJSON_Print(action);
            LEDOption->value = atoi(msg);
            signal *= -1;
        }
        printf("Receive message from Server : %s\n", msg);
        free(msg);
        free(receiveJSON);
        sleep(atoi(argv[3]));
        close(sock);
    }
    free(getLightThread);
    free(LEDThread);
    printf("Program is downed. please rerun\n");
    return 0;
}
