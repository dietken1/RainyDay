# 컴파일러
CC = cc

# 컴파일러 플래그
CFLAGS = -g

# 링크할 라이브러리
LIBS = -lpthread -lcjson

# 소스 파일
SRCS = main.c getLight.c led.c

# 객체 파일
OBJS = $(SRCS:.c=.o)

# 실행 파일
TARGET = main

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# 개별 소스 파일 컴파일
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 클린업
clean:
	rm -f $(OBJS) $(TARGET)

# 디펜던시 파일 생성
depend: $(SRCS)
	makedepend $(SRCS)

# 디펜던시 파일 포함
-include $(SRCS:.c=.d)

.PHONY: all clean depend
