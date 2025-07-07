#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

// 主函数
int main() {
    printf("=== ARMv7 Demo Program ===\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("目标架构: ARMv7\n\n");
    
    // 测试数学运算
    int a = 10, b = 5;
    printf("数学运算测试AAA:\n");
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d * %d = %d\n", a, b, multiply(a, b));
    printf("%.2f ^ %.2f = %.2f\n", 2.0, 3.0, power(2.0, 3.0));
    
    // 测试字符串处理
    char message[] = "Hello ARMv7!";
    printf("\n字符串处理测试:\n");
    printf("原始字符串: %s\n", message);
    reverse_string(message);
    printf("反转后: %s\n", message);
    
    printf("\n程序执行完成!\n");
    return 0;
} 