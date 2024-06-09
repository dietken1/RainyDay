#include "threadh.h"
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <sys/ioctl.h>

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 40
#define DIRECTION_MAX 35
#define BUFFER_MAX 3

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

static int prepare(int fd)
{
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1)
    {
        perror("Can't set MODE");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1)
    {
        perror("Can't set number of BITS");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1)
    {
        perror("Can't set write CLOCK");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1)
    {
        perror("Can't set read CLOCK");
        return -1;
    }

    return 0;
}

uint8_t control_bits_differential(uint8_t channel)
{
    return (channel & 7) << 4;
}

uint8_t control_bits(uint8_t channel)
{
    return 0x8 | control_bits_differential(channel);
}

int readadc(int fd, uint8_t channel)
{
    uint8_t tx[] = {1, control_bits(channel), 0};
    uint8_t rx[3];

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = ARRAY_SIZE(tx),
        .delay_usecs = DELAY,
        .speed_hz = CLOCK,
        .bits_per_word = BITS,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1)
    {
        perror("IO Error");
        abort();
    }

    return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}

// 쓰레드 cancel시 호출될 함수
static void dispose(void *option)
{
    OPTION *optionInfo;
    optionInfo = (OPTION *)option;
    // 할당한 자원 해제

    free(option);
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
