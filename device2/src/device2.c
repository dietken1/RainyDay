#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "cjson/cJSON.h"
#include "../header/speaker.h"
#include "../header/temperature_and_humidity.h"

// function for handling error
void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

// function for creating and sending json object
void create_and_send_json(int socket_fd, int value_from_sensor, int flag) {
    cJSON *json = cJSON_CreateObject();

    // handling error: json object is not created
    if (json == NULL) {
        printf("Failed to create JSON object\n");
        return;
    }

    /*
    json: add data
    when flag is 1, which means this json object is generated for sending data from sensor to main device */
    if (flag){
        cJSON_AddStringToObject(json, "type", "sensor");
        cJSON_AddStringToObject(json, "sensor_type", "humidity");
        cJSON_AddNumberToObject(json, "value", value_from_sensor);
    }
    
    // when flag is 0, which means this json object is generated for sending request to main device
    else{
        cJSON_AddStringToObject(json, "type", "actuator");
        cJSON_AddStringToObject(json, "actuator_type", "speaker");
    }

    // convert json object into string
    char *json_str = cJSON_PrintUnformatted(json);
    if (json_str == NULL) {
        printf("Failed to print JSON object\n");
        cJSON_Delete(json);
        return;
    }

    // send json string
    send(socket_fd, json_str, strlen(json_str), 0);

    // when sending data
    if (flag)
        printf("Sent data: %s\n", json_str);
    
    // when sending request
    else
        printf("Sent request: %s\n", json_str);

    // return memory and clear json
    free(json_str);
    cJSON_Delete(json);
}

int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    char msg[1024] = {0};
    int str_len;
    int data_to_send = 0;

    cJSON *json;
    cJSON *type;
    const cJSON *actuator_type;
    const cJSON *action;

    // checking if received appropriate parameters or not
    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // noticing there is no problem with connecting to user
    printf("Connection established\n");

    // For temperature and humidity sensor
    while (1) {
        // create new socket
        sock = socket(PF_INET, SOCK_STREAM, 0);

        // handling error: socket is not created
        if (sock == -1)
            error_handling("socket() error");

        // initialize struct serv_addr as 0
        memset(&serv_addr, 0, sizeof(serv_addr));

        // set IP version: the socket will use IPv4 address
        serv_addr.sin_family = AF_INET;

        // convert string type ip address into binary type
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

        // convert string type port number into integer type
        serv_addr.sin_port = htons(atoi(argv[2]));

        // handling error: no connection
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
            error_handling("connect() error");

        // receive data from the sensor
        data_to_send = t_and_h();

        // send data to main device
        create_and_send_json(sock, data_to_send, 1);

        // time pause for 2 sec
        usleep(2000000);

        // receive response from main device
        str_len = read(sock, msg, sizeof(msg)-1);

        // handling error: not reading response
        if (str_len == -1)
            error_handling("read() error");

        // put null character at end of response
        msg[str_len] = '\0';

        // reading json formatted response
        json = cJSON_Parse(msg);

        // handling error: json object is null
        if (json == NULL){
            fprintf(stderr, "Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
            continue;
        }

        // reading json object and handling error
        type = cJSON_GetObjectItem(json, "status");
        if (cJSON_IsString(type) && (type->valuestring != NULL)) {
            if (strcmp(type->valuestring, "ok") == 0)
                printf("response : %s\n", json);

            // there's only one response from main device
            else {
                printf("received json object doesn't have value\n");
                continue;
            }
        }

        else {
            printf("received json object is null or not string data\n");
            continue;
        }

        // clear json, msg; and close socket
        cJSON_Delete(json);
        memset(msg, 0, sizeof(msg));
        close(sock);

        // time pause for 0.5 sec
        usleep(500000);

        /*
        For speaker(buzzer)
        create new socket */
        sock = socket(PF_INET, SOCK_STREAM, 0);

        // handling error: socket is not created
        if (sock == -1)
            error_handling("socket() error");

        // initialize struct serv_addr as 0
        memset(&serv_addr, 0, sizeof(serv_addr));

        // set IP version: the socket will use IPv4 address
        serv_addr.sin_family = AF_INET;

        // convert string type ip address into binary type
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

        // convert string type port number into integer type
        serv_addr.sin_port = htons(atoi(argv[2]));

        // handling error: no connection
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
            error_handling("connect() error");

        // create json object for sending request to main device
        create_and_send_json(sock, 0, 0);

        // time pause for 2 sec
        usleep(2000000);

        // read response from main device
        str_len = read(sock, msg, sizeof(msg)-1);

        // handling error: not reading response
        if (str_len == -1)
            error_handling("read() error");

        // put null character at the end of response
        msg[str_len] = '\0';

        // read json object
        json = cJSON_Parse(msg);

        // handling error: json object is null
        if (json == NULL){
            fprintf(stderr, "Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
            continue;
        }

        // reading json object and handling error
        type = cJSON_GetObjectItem(json, "action");
        if (cJSON_IsNumber(type)) {
            if (type->valueint == 1){
                printf("instruction : %s\n", json);
                buzzer();
            }

            else if (type->valueint == 0)
                printf("instruction : %s\n", json);
            
            else
                printf("received json object has no value\n");
        }

        else {
            printf("received json object is null or not numerical data\n");
            continue;
        }

        // clear json object, msg; and close socket
        cJSON_Delete(json);
        memset(msg, 0, sizeof(msg));
        close(sock);

        /* 
        time pause for 3 sec
        since the humidity doesn't change rapidly */
        usleep(3000000);
    
    }

    return 0;
}
