#include <stdio.h>
#include "drivers\input\keyboard\matrix.h"

int main(void) {
    printf("Hello, World!\n");
    return 0;
}

// 系统调用 stub（用于嵌入式）
void _exit(int code) {
    (void)code;
    while(1) {
        // 无限循环，MCU程序不会退出
    }
}
