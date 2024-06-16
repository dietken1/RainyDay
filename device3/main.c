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

#define BUFFER_MAX 20
#define DIRECTION_MAX 256
#define VALUE_MAX 256

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

pthread_t *getLightThread;
pthread_t *LEDThread;
char buffer[1024] = {0};

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

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

    message = cJSON_Print(json);
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
    struct sockaddr_in serv_addr;
    int light = 0;
    char *message;
    int signal = 1;
    char *msg;

    if (argc != 3) // != 4
    {
        printf("Usage : %s <IP> <port>\n", argv[0]); // printf("Usage : %s <IP> <port> <polling_rate>\n", argv[0]);
        exit(1);
    }

    OPTION *getLightOption = (OPTION *)calloc(1, sizeof(OPTION));
    strcpy(getLightOption->type, "sensor");
    strcpy(getLightOption->devName, "light_intensity");
    getLightOption->polling_rate = 100; // atoi(argv[3]);
    getLightOption->value = 0;

    OPTION *LEDOption = (OPTION *)calloc(1, sizeof(OPTION));
    strcpy(LEDOption->type, "actuator");
    strcpy(LEDOption->devName, "led");
    LEDOption->polling_rate = 100; // atoi(argv[3]);
    LEDOption->value = 0;

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

        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock == -1)
            error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
        serv_addr.sin_port = htons(atoi(argv[2]));

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
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
        sleep(1);
        close(sock);
    }
    free(getLightThread);
    free(LEDThread);
    printf("Program is downed. please rerun\n");
    return 0;
}
