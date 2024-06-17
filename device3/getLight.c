#include "threadh.h"
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <sys/ioctl.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) // // 배열의 크기 계산 매크로

static const char *DEVICE = "/dev/spidev0.0";  // 사용할 SPI 장치 파일 경로
static uint8_t MODE = 0;  // SPI 모드 설정 (0)
static uint8_t BITS = 8;  // SPI 비트 설정 (8비트)
static uint32_t CLOCK = 1000000;  // SPI 클록 속도 설정 (1MHz)
static uint16_t DELAY = 5;  // SPI 전송 지연시간 설정 (5us)

static int prepare(int fd)
{
    // SPI 모드 설정
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1)
    {
        perror("Can't set MODE");
        return -1;
    }

    // SPI 비트 수 설정
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1)
    {
        perror("Can't set number of BITS");
        return -1;
    }

    // SPI 쓰기 클록 속도 설정
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1)
    {
        perror("Can't set write CLOCK");
        return -1;
    }

    // SPI 읽기 클록 속도 설정
    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1)
    {
        perror("Can't set read CLOCK");
        return -1;
    }

    return 0;
}

// 차동 제어 비트
uint8_t control_bits_differential(uint8_t channel)
{
    return (channel & 7) << 4; // channel의 하위 3비트만 남기고 나머지 비트를 0으로 만든 후, 4비트 이동.
}

// 제어 비트 설정
uint8_t control_bits(uint8_t channel)
{
    return 0x8 | control_bits_differential(channel); // 16진수 8을 차동 제어 비트로 만듦.
}

// ADC 읽기 함수
int readadc(int fd, uint8_t channel)
{
    uint8_t tx[] = {1, control_bits(channel), 0}; // 전송 데이터 배열
    uint8_t rx[3]; // 수신 데이터 배열

    // SPI 전송 구조체
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,  // 전송 버퍼 주소
        .rx_buf = (unsigned long)rx,  // 수신 버퍼 주소
        .len = ARRAY_SIZE(tx),  // 버퍼 길이
        .delay_usecs = DELAY,  // 지연 시간
        .speed_hz = CLOCK,  // 클록 속도
        .bits_per_word = BITS,  // 비트 수
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) // SPI 메시지 전송
    {
        perror("IO Error");
        abort();
    }

    return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF); // 수신 데이터 처리하여 반환 -> rx[1]의 상위 2비트와 rx[2]의 하위 8비트를 결합하여 10비트 ADC 값을 반환
}

// 쓰레드 cancel시 호출될 함수
static void dispose(void *option)
{
    OPTION *optionInfo;
    optionInfo = (OPTION *)option;
    // 할당한 자원 해제

    free(optionInfo);
}

// 폴링으로 버튼의 이벤트를 처리하는 함수
void *get_light_routine(void *option)
{
    OPTION *optionInfo;

    int fd = open(DEVICE, O_RDWR);
    if (fd <= 0)
    {
        perror("Device open error");
        return 0;
    }

    if (prepare(fd) == -1)
    {
        perror("Device prepare error");
        return 0;
    }

    optionInfo = (OPTION *)option;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cleanup_push(dispose, option);

    // 폴링하면서 버튼 이벤트 핸들링.
    while (1)
    {
        optionInfo->value = readadc(fd, 0);
        // 폴링레이트에 따라서 1초에 몇번 작동할지에 따라, 해당하는 HZ로 동작하도록 usleep 함.
        usleep(1000000 / optionInfo->polling_rate);
    }
    close(fd);
    pthread_cleanup_pop(0);
}

pthread_t *initGetLight(OPTION* optionInfo)
{
    pthread_t *buttonT;

    buttonT = malloc(sizeof(pthread_t));

    if (pthread_create(buttonT, NULL, get_light_routine, (void *)optionInfo) == 0)
        return buttonT;
    else
        return (NULL);
}
