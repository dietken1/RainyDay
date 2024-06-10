# system programming and practice: Team project
Team name: RainyDay


# Device 2
Author: Junsung Kim


# Description of device 2
Device2 controls 1 sensor and 1 actuator

Sensor: temperature and humidity sensor (dht11)

Actuator: speaker (piezo buzzer)


# For using sensor
The order of pin: vcc(+) - i/o - ground(-)

The written code uses GPIO 4


# For using actuator
The order of pin: vcc(+) - i/o - ground(-)

The written code uses GPIO 18


# To execute device2
Notice: need sudo permission for executing code

~/device2 $ cd exe

~/device2/exe $ gcc -c ../src/speaker.c

~/device2/exe $ gcc -c ../src/temperature_and_humidity.c

~/device2/exe $ gcc -I/usr/local/include/cjson ../src/device2.c -L/usr/local/lib/arm-linux-gnueabihf -lcjson -c

export LD_LIBRARY_PATH=/usr/local/lib/arm-linux-gnueabihf:$LD_LIBRARY_PATH

~/device2 $ gcc -o device2 -I/usr/local/include/cjson device2.o speaker.o temperature_and_humidity.o -L/usr/local/lib/arm-linux-gnueabihf -lcjson -l wiringPi

~/device2 $ sudo LD_LIBRARY_PATH=/usr/local/lib/arm-linux-gnueabihf:$LD_LIBRARY_PATH ./device2 IP PORT
