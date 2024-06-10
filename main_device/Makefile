# 컴파일러와 플래그 설정
CC = gcc
CFLAGS = -I./include -I./include/services -Wall -Wextra -pthread
LDFLAGS = -pthread -li2c -lcjson

# 소스 파일과 오브젝트 파일 설정
SRCS = main.c $(wildcard src/*.c src/services/*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

# 실행 파일 이름
TARGET = RainyDay

# 기본 타겟 설정
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# 개별 소스 파일에 대한 오브젝트 파일 생성 규칙
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 정리 규칙
clean:
	rm -f $(OBJS) $(TARGET)

# 디펜던시 파일 포함
-include $(SRCS:.c=.d)

# 디펜던시 파일 생성 규칙
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
