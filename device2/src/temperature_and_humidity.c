#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
can modify these numbers
GPIO 4 */
#define T_H_PIN 7

/*
macro variable: for storing data from sensor
the data will be stored as below:
    data[0]: integer part of humidity data
    data[1]: float part of humidity data
    data[2]: integer part of temperature data
    data[3]: float part of temperature data
    data[4]: checksum (to verify the integrity of the data) */
int data[5];

// function: for reading data from sensor
int readData(void){
    uint8_t laststate = HIGH;
    uint8_t cnt = 0;
    uint8_t j = 0;

    // initialize data array: set all elements as 0
    for (int i = 0; i < 5; i++)
        data[i] = 0;

    // set pin mode to output mode: for sending signals to sensor
    pinMode(T_H_PIN, OUTPUT);

    // initiate communication with sensor by setting the pin low for 18 ms
    digitalWrite(T_H_PIN, LOW);
    delay(18);

    // send signals to sensor for preparing data transmission by setting the pin high for 40 ms
    digitalWrite(T_H_PIN, HIGH);
    delayMicroseconds(40);

    // set pin mode to input mode: for receiving data from sensor
    pinMode(T_H_PIN, INPUT);

    // read data from sensor: uint8_t for saving memory space
    for (uint8_t i = 0; i < 100; i++){
        // initialize cnt with 0 
        cnt = 0;

        // repeat till changing status of pin
        while (digitalRead(T_H_PIN) == laststate){
            cnt++;

            // time delay for 1 ms
            delayMicroseconds(1);

            // when not changed 255 times
            if (cnt == 255)
                break;
        }

        // store present status of pin
        laststate = digitalRead(T_H_PIN);

        // when not changed for 255 times
        if (cnt == 255)
            break;

        // skip first 3 data (for filtering noise), and then read data bits
        if ((i >= 4) && (i % 2 == 0)){
            // left shifting 1 time
            data[j/8] <<= 1;

            // set 1 if cnt is greater than 16
            if (cnt > 16)
                data[j/8] |= 1;
            j++;
        }
    }

    // Verify the integrity of the data by comparing checksum
    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))){
        // valid data
        return 1;
    }

    // invalid data
    return 0;
}

int t_and_h(void){
    // initialize wiringPi library to control GPIO pin: repeat till success
    while (wiringPiSetup() == -1);

    // read data: repeat till receiving valid data
    while (readData() == 0)
        delay(1000);

    // return integer part of humidity only without ceiling
    return data[0];
}
