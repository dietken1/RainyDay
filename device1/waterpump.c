#include <wiringPi.h>    //wiringPi 라이브러리
#include <arpa/inet.h>   //네트워크 소켓을 위한 라이브러리
#include <json-c/json.h> //JSON 라이브러리
#include <stdio.h>       //표준 입출력 함수 사용 목적
#include <stdlib.h>      //표준 라이브러리
#include <string.h>      //문자열 처리 라이브러리
#include <sys/socket.h>  //소켓 통신 라이브러리
#include <unistd.h>      //표준 심볼 상수 및 자료형 사용 목적

#define PIN 0 // PIN:0 (wiringPi GPIO 17)

//서버에 펌프 상태를 요청하는 함수
int askStatus(int sck) {
    char response[256];     //서버로부터 응답을 받을 버퍼

    //JSON형식의 Request 객체 생성
    struct json_object* request_json = json_object_new_object();                                 //객체 생성
    json_object_object_add(request_json, "type", json_object_new_string("actuator"));            //요청에 "type": "actuator" 추가
    json_object_object_add(request_json, "actuator_type", json_object_new_string("water_pump")); //요청에 "actuator_type": "water_pump" 추가
    const char* request_str = json_object_to_json_string(request_json);                          //JSON 객체를 문자열로 변환해주는 함수

    //Request
    if (write(sck, request_str, strlen(request_str)) == -1) {  //서버에 Request전송
        perror("Write Error");
        json_object_put(request_json);  //JSON 객체를 해제해주는 함수
        abort();
    }
    json_object_put(request_json);

    //Response
    int str_len = read(sck, response, sizeof(response) - 1); //서버로부터 받은 Response를 읽음
    if (str_len == -1) {
        perror("Read Error");
        close(sck);
        abort();
    }
    response[str_len] = '\0'; // 응답의 끝에 null 문자를 추가하여 문자열로 만듦

    //Response->Json 파싱
    struct json_object* parsed_json; //파싱된 JSON 객체
    struct json_object* action;      //JSON 객체 내의 action 필드

    parsed_json = json_tokener_parse(response); //응답을 파싱하여 JSON 객체로 변환
    if (parsed_json == NULL) {
        fprintf(stderr, "Parsing Error\n");
        close(sck);
        abort();
    }

    if (!json_object_object_get_ex(parsed_json, "action", &action)) { //변환한 객체에서 action필드 추출
        fprintf(stderr, "Field Error\n");
        json_object_put(parsed_json);
        close(sck);
        abort();
    }

    int action_value = json_object_get_int(action);     //action필드에서 정수 값 추출

    if (action_value == 1 || action_value == 0) {
        printf("Server Response: %d\n", action_value);  //통신 성공
    }
    else {
        printf("Server Response: ERROR\n");             //통신 실패
    }

    json_object_put(parsed_json);                       //JSON 객체 해제
    return action_value;                                //펌프 작동여부 반환
}

//메인 함수
int main(int argc, char* argv[]) {

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    int sck; // 소켓 파일 디스크립터
    struct sockaddr_in serv_addr; // 서버 주소 구조체

    //WiringPi 설정 초기화
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "WiringPi Error\n");
        return -1;
    }

    pinMode(PIN, OUTPUT); //핀을 출력모드로 설정

    while (1) {
        sck = socket(PF_INET, SOCK_STREAM, 0); //소켓 생성
        if (sck == -1) {
            perror("Socket Error");
            sleep(10000000);
            continue;
        }

        memset(&serv_addr, 0, sizeof(serv_addr));       //서버 주소 구조체 초기화
        serv_addr.sin_family = AF_INET;                 //주소 체계 설정
        serv_addr.sin_addr.s_addr = inet_addr(argv[1]); //서버 IP 주소 설정
        serv_addr.sin_port = htons(atoi(argv[2]));      //서버 포트 번호 설정

        if (connect(sck, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) { //서버에 연결
            perror("Connection Error");
            close(sck);
            sleep(10000000); //잠시 대기
            continue;
        }

        int action = askStatus(sck); //서버에 상태 요청

        if (action == 1) {           //작동
            digitalWrite(PIN, HIGH);
            printf("Pump ON\n");
            delay(1000);
            digitalWrite(PIN, LOW);
        }
        else {                       //중지
            digitalWrite(PIN, LOW);
            printf("Pump OFF\n");
        }

        close(sck);     //소켓 닫기
        delay(10000);   //10초 대기(서버에게 응답요청 메세지를 보내는 주기)
    }

    return 0; //프로그램 종료
}
