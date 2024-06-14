#include <wiringPi.h>
#include <stdio.h>

/*
can modify these numbers
GPIO 18 */
#define PIN 1

// function for speaker(buzzer)
int buzzer(void){
    // initialize wiringPi library to control GPIO pin: repeat till success
    while (wiringPiSetup() == -1);

    // set GPIO pin to pwm mode
    pinMode(PIN, PWM_OUTPUT);

    /*
    set pwm clock
    19.2 MHz(default) / 19 = about 1 MHz */
    pwmSetClock(19);

    /*
    set pwm mode to Mark-Space mode for more accurate pwm signal
    Mark-Space mode is used for controlling the desired frequency by
    using ON/OFF cycle
    default: balanced mode */
    pwmSetMode(PWM_MODE_MS);

    // activate speaker 3 times
    for (int i = 0; i < 3; i++){
        // time pause for 1 sec
        delay(1000);

        /*
        set pwm range to 262 hz (pitch C)
        clock / pwmSetRange parameter */
        pwmSetRange(1000000 / 262);

        /*
        set pwm duty cycle
        50% on, 50% off for clear sound
        clock / pwmWrite second parameter */
        pwmWrite(PIN, 1000000 / 262 / 2);

        // ringing for 3 sec
        delay(3000);

        // Stop ringing by setting the duty cycle to 0 (OFF)
        pwmWrite(PIN, 0);
    }
}

