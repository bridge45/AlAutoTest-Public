#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// QuickJS 头文件
#include "quickjs.h"

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

// 读取文件内容
char* read_file_content(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    fclose(file);
    
    return content;
}

// console.log 实现
static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    for (int i = 0; i < argc; i++) {
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            printf("%s", str);
            JS_FreeCString(ctx, str);
        }
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return JS_UNDEFINED;
}

// 执行 JavaScript 代码
int execute_javascript(const char* js_code, const char* filename) {
    JSRuntime* rt = JS_NewRuntime();
    if (!rt) {
        fprintf(stderr, "无法创建 JS 运行时\n");
        return -1;
    }
    
    JSContext* ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "无法创建 JS 上下文\n");
        JS_FreeRuntime(rt);
        return -1;
    }
    
    // 设置模块加载器
    JS_SetModuleLoaderFunc(rt, NULL, NULL, NULL);
    
    // 添加 console 对象
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue console_obj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, console_obj, "log", JS_NewCFunction(ctx, js_console_log, "log", 1));
    JS_SetPropertyStr(ctx, global_obj, "console", console_obj);
    JS_FreeValue(ctx, global_obj);
    
    // 执行 JavaScript 代码
    JSValue val = JS_Eval(ctx, js_code, strlen(js_code), filename, JS_EVAL_TYPE_GLOBAL);
    
    if (JS_IsException(val)) {
        JSValue exception = JS_GetException(ctx);
        const char* error_str = JS_ToCString(ctx, exception);
        fprintf(stderr, "JavaScript 执行错误: %s\n", error_str);
        JS_FreeCString(ctx, error_str);
        JS_FreeValue(ctx, exception);
        JS_FreeValue(ctx, val);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return -1;
    }
    
    // 获取执行结果
    if (!JS_IsUndefined(val)) {
        const char* result_str = JS_ToCString(ctx, val);
        if (result_str) {
            printf("JavaScript 执行结果: %s\n", result_str);
            JS_FreeCString(ctx, result_str);
        }
    }
    
    JS_FreeValue(ctx, val);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 0;
}

// 主函数
int main() {
    printf("=== ARMv7 Demo Program with QuickJS ===\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("目标架构: ARMv7\n\n");
    
    // 测试数学运算
    int a = 10, b = 5;
    printf("C 语言数学运算测试:\n");
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d * %d = %d\n", a, b, multiply(a, b));
    printf("%.2f ^ %.2f = %.2f\n", 2.0, 3.0, power(2.0, 3.0));
    
    // 测试字符串处理
    char message[] = "Hello ARMv7!";
    printf("\nC 语言字符串处理测试:\n");
    printf("原始字符串: %s\n", message);
    reverse_string(message);
    printf("反转后: %s\n", message);
    
    // 测试 QuickJS
    printf("\n=== QuickJS 测试 ===\n");
    
    // 读取并执行 test.js
    char* js_content = read_file_content("test.js");
    if (js_content) {
        printf("执行 test.js 文件:\n");
        if (execute_javascript(js_content, "test.js") == 0) {
            printf("JavaScript 执行成功!\n");
        } else {
            printf("JavaScript 执行失败!\n");
        }
        free(js_content);
    } else {
        printf("无法读取 test.js 文件，执行内联 JavaScript 测试:\n");
        
        // 执行内联 JavaScript 代码
        const char* inline_js = 
            "console.log('内联 JavaScript 测试');"
            "console.log('2 + 3 =', 2 + 3);"
            "console.log('Math.PI =', Math.PI);"
            "console.log('Hello from QuickJS!');";
        
        if (execute_javascript(inline_js, "<inline>") == 0) {
            printf("内联 JavaScript 执行成功!\n");
        } else {
            printf("内联 JavaScript 执行失败!\n");
        }
    }
    
    printf("\n程序执行完成!\n");
    return 0;
} 