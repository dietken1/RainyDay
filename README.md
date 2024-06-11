# system programming and practice: Team project
Team name: RainyDay

# Main Device
### 명령어

1. **실행**

```sh
sudo ./[프로젝트 dir]
```

2. **컴파일**

```sh
make
```

3. **컴파일된 오브젝트 파일과 실행 파일을 제거**

```sh
make clean
```

# Device 1
### waterpump.c 소개

Main Device에 작동 여부를 요청하고, 응답 값을 바탕으로 워터펌프를 작동시키는 코드

### waterpump.c 컴파일방법

``` sh
gcc -o waterpump waterpump.c -lwiringPi -ljson-c
```

### waterpump.c 실행방법

``` sh
./waterpump IP주소 PORT번호
```

### waterlevel.c 소개

수위센서를 작동시켜 센서값을 디지털 값으로 변환한 후, Main Device에 전송하는 코드

### waterlevel.c 컴파일방법

``` sh
gcc -o waterlevel watelevel.c -ljson-c
```

### waterlevel.c 실행방법

``` sh
./waterlevel IP주소 PORT번호
```

# Device 2

### Description of device 2
Device2 controls 1 sensor and 1 actuator

Sensor: temperature and humidity sensor (dht11)

Actuator: speaker (piezo buzzer)


### For using sensor
The order of pin: vcc(+) - i/o - ground(-)

The written code uses GPIO 4


### For using actuator
The order of pin: vcc(+) - i/o - ground(-)

The written code uses GPIO 18


### To execute device2
Notice: need sudo permission for executing code

``` sh
cd exe

gcc -c ../src/speaker.c

gcc -c ../src/temperature_and_humidity.c

gcc -I/usr/local/include/cjson ../src/device2.c -L/usr/local/lib/arm-linux-gnueabihf -lcjson -c

export LD_LIBRARY_PATH=/usr/local/lib/arm-linux-gnueabihf:$LD_LIBRARY_PATH

gcc -o device2 -I/usr/local/include/cjson device2.o speaker.o temperature_and_humidity.o -L/usr/local/lib/arm-linux-gnueabihf -lcjson -l wiringPi

sudo LD_LIBRARY_PATH=/usr/local/lib/arm-linux-gnueabihf:$LD_LIBRARY_PATH ./device2 IP PORT
```

# Device 3

### 실행 방법

```bash
make

./main 서버IP주소 서버포트
```
