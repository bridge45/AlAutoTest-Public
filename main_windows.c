#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 简单的数学运算函数
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

double power(double base, double exponent) {
    return pow(base, exponent);
}

// 字符串处理函数
void reverse_string(char* str) {
    int length = strlen(str);
    for (int i = 0; i < length / 2; i++) {
        char temp = str[i];
        str[i] = str[length - 1 - i];
        str[length - 1 - i] = temp;
    }
}

// 获取当前时间字符串
void get_current_time(char* time_str) {
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    strftime(time_str, 50, "%Y-%m-%d %H:%M:%S", local_time);
}

// 主函数
int main() {
    char time_str[50];
    get_current_time(time_str);
    
    printf("=== Windows Demo Program ===\n");
    printf("编译时间: %s\n", __DATE__);
    printf("运行时间: %s\n", time_str);
    printf("目标架构: Windows x86_64\n\n");
    
    // 测试数学运算
    int a = 10, b = 5;
    printf("数学运算测试:\n");
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d * %d = %d\n", a, b, multiply(a, b));
    printf("%.2f ^ %.2f = %.2f\n", 2.0, 3.0, power(2.0, 3.0));
    
    // 测试字符串处理
    char message[] = "Hello Windows!";
    printf("\n字符串处理测试:\n");
    printf("原始字符串: %s\n", message);
    reverse_string(message);
    printf("反转后: %s\n", message);
    
    // Windows 特定功能
    #ifdef _WIN32
    printf("\nWindows 系统信息:\n");
    printf("当前目录: ");
    char cwd[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, cwd)) {
        printf("%s\n", cwd);
    }
    #endif
    
    printf("\n程序执行完成!\n");
    printf("按任意键退出...\n");
    getchar();
    return 0;
} 