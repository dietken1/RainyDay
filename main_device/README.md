## Main device 프로젝트 구조

- service dir로 비지니스 로직과 인프라로직을 분리하였습니다.
- 도메인 로직과 비지니스 로직을 굳이 구분하진 않았습니다.
- config.txt에는 button gpio 핀번호와 pr이 있습니다.

```bash
main_device/
├── include/                      # 헤더 파일 디렉터리
│   ├── config.h
│   ├── gpio.h
│   ├── LCD1602.h
│   ├── socket.h
│   ├── button.h
│   └── services/
│       ├── button_service.h
│       ├── communication_service.h
│       ├── lcd1602_service.h
│       ├── plant.h
│       └── step.h
├── src/                          # 소스 파일 디렉터리
│   ├── config.c
│   ├── gpio.c
│   ├── LCD1602.c
│   ├── socket.c
│   ├── button.c
│   └── services/
│       ├── button_service.c
│       ├── communication_service.c
│       ├── lcd1602_service.c
│       ├── plant.c
│       └── step.c
├── config.txt                   # 버튼 스레드 pr 및 gpio 핀번호
├── Makefile
├── README.md
└── main.c
```

## 요구 사항

빌드하고 실행하려면 다음과 같은 라이브러리가 필요합니다

- `gcc` (GNU Compiler Collection)
- `make`
- `pthread` (POSIX 스레드)
- `i2c` 라이브러리
- `cjson` 라이브러리

### 설치(linux)

다음 명령어를 사용하여 필요한 라이브러리를 설치할 수 있습니다

   ```bash
   sudo apt update
   sudo apt install build-essential make libi2c-dev libcjson-dev
   ```

### 명령어

1. **실행**

   ```sh
   sudo ./[프로젝트 dir]
   
2. **컴파일**

   ```sh
   make

3. **컴파일된 오브젝트 파일과 실행 파일을 제거**

   ```sh
   make clean