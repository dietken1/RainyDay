#include "threadh.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>  

#define IN 0
#define OUT 1
#define PWM 0

#define LOW 0
#define HIGH 1
#define VALUE_MAX 256
#define DIRECTION_MAX 256

#define PERIOD 10000000 // 100hz
#define DUTY_CYCLE PERIOD / 100 // 1hz

static int PWMExport(int pwmnum)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    int fd, byte;

    // TODO: Enter the export path.
    fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open export for export!\n");
        return (-1);
    }

    byte = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
    write(fd, buffer, byte);
    close(fd);

    sleep(1);

    return (0);
}

static int PWMEnable(int pwmnum)
{
    static const char s_enable_str[] = "1";

    char path[DIRECTION_MAX];
    int fd;

    // TODO: Enter the enable path.
    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm0/enable", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in enable!\n");
        return -1;
    }

    write(fd, s_enable_str, strlen(s_enable_str));
    close(fd);

    return (0);
}

static int PWMWritePeriod(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;

    // TODO: Enter the period path.
    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm0/period", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in period!\n");
        return (-1);
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_value_str, byte))
    {
        fprintf(stderr, "Failed to write value in period!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return (0);
}

static int PWMWriteDutyCycle(int pwmnum, int value)
{
    char s_value_str[VALUE_MAX];
    char path[VALUE_MAX];
    int fd, byte;

    // TODO: Enter the duty_cycle path.
    snprintf(path, VALUE_MAX, "/sys/class/pwm/pwmchip0/pwm0/duty_cycle", pwmnum);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open in duty cycle!\n");
        return (-1);
    }
    byte = snprintf(s_value_str, VALUE_MAX, "%d", value);

    if (-1 == write(fd, s_value_str, byte))
    {
        fprintf(stderr, "Failed to write value in duty cycle!\n");
        close(fd);
        return -1;
    }
    close(fd);

    return (0);
}

// 쓰레드 cancel시 호출될 함수
static void dispose(void *option)
{
    OPTION *optionInfo;
    optionInfo = (OPTION *)option;
    // 할당한 자원 해제

    free(option);
}

void adjustLED(int *currentLEDValue, int LEDValue)
{

    assert(LEDValue >= 0 && LEDValue <= 100);
    while (1)
    {
        if (*currentLEDValue == LEDValue)
            return;
        else if (*currentLEDValue < LEDValue)
            *currentLEDValue += 1;
        else if (*currentLEDValue > LEDValue)
            *currentLEDValue -= 1;

        PWMWriteDutyCycle(PWM, *currentLEDValue * DUTY_CYCLE);
        usleep(10000);
    }
}

// 폴링으로 버튼의 이벤트를 처리하는 함수
void *led_routine(void *option)
{
    OPTION *optionInfo;
    int currentLEDValue;

    optionInfo = (OPTION *)option;
    currentLEDValue = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(dispose, option);

    // 폴링하면서 버튼 이벤트 핸들링.
    while (1)
    {
        // 폴링레이트에 따라서 1초에 몇번 작동할지에 따라, 해당하는 HZ로 동작하도록 usleep 함.
        if (optionInfo->value != currentLEDValue)
            adjustLED(&currentLEDValue, optionInfo->value);
        usleep(1000000 / optionInfo->polling_rate);
    }

    pthread_cleanup_pop(0);
}

pthread_t *initLED(OPTION* optionInfo)
{
    pthread_t *buttonT;

    buttonT = malloc(sizeof(pthread_t));

    PWMExport(PWM);
    PWMWritePeriod(PWM, PERIOD); // 주기
    PWMWriteDutyCycle(PWM, 0);
    PWMEnable(PWM);

    if (pthread_create(buttonT, NULL, led_routine, (void *)optionInfo) == 0)
        return buttonT;
    else
        return (NULL);
}
