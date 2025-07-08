#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/wait.h>

// QuickJS 头文件（条件编译）
#ifdef QUICKJS_AVAILABLE
#include "quickjs.h"
#endif

// BearSSL 头文件（条件编译）
#ifdef BEARSSL_AVAILABLE
#include "bearssl.h"
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

#ifdef BEARSSL_AVAILABLE
// HTTPS 请求函数
char* https_request(const char* host, const char* path, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "无法创建socket\n");
        return NULL;
    }
    
    struct hostent *server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "无法解析主机名: %s\n", host);
        close(sock);
        return NULL;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "连接失败\n");
        close(sock);
        return NULL;
    }
    
    // 构建HTTP请求
    char request[1024];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n", path, host);
    
    if (send(sock, request, strlen(request), 0) < 0) {
        fprintf(stderr, "发送请求失败\n");
        close(sock);
        return NULL;
    }
    
    // 接收响应
    char buffer[4096];
    char* response = malloc(8192);
    response[0] = '\0';
    int total_size = 0;
    
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        strcat(response, buffer);
        total_size += bytes_received;
        
        if (total_size > 8000) break; // 防止缓冲区溢出
    }
    
    close(sock);
    return response;
}
#else
// BearSSL不可用时的占位函数
char* https_request(const char* host, const char* path, int port) {
    fprintf(stderr, "BearSSL 功能不可用\n");
    return NULL;
}
#endif

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

#ifdef QUICKJS_AVAILABLE
// console.log 实现
static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val; // 明确表示不使用此参数
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

// https_request 实现
static JSValue js_https_request(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val;
    if (argc < 3) {
        return JS_EXCEPTION;
    }
    
    const char* host = JS_ToCString(ctx, argv[0]);
    const char* path = JS_ToCString(ctx, argv[1]);
    int port;
    JS_ToInt32(ctx, &port, argv[2]);
    
    if (!host || !path) {
        if (host) JS_FreeCString(ctx, host);
        if (path) JS_FreeCString(ctx, path);
        return JS_EXCEPTION;
    }
    
    char* response = https_request(host, path, port);
    JS_FreeCString(ctx, host);
    JS_FreeCString(ctx, path);
    
    if (response) {
        JSValue result = JS_NewString(ctx, response);
        free(response);
        return result;
    }
    
    return JS_NULL;
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
    
    // 添加 https_request 函数到全局对象
    JS_SetPropertyStr(ctx, global_obj, "https_request", 
        JS_NewCFunction(ctx, js_https_request, "https_request", 3));
    
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
#else
// 当QuickJS不可用时的占位函数
int execute_javascript(const char* js_code, const char* filename) {
    printf("QuickJS 功能不可用，跳过 JavaScript 执行\n");
    return 0;
}
#endif

// 主函数
int main() {
    printf("=== ARMv7 Demo Program with QuickJS ===\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("目标架构: ARMv7\n\n");
    
    char version[32] = "unknown";
    FILE *vf = fopen("../version", "r");
    if (!vf) vf = fopen("version", "r");
    if (vf) {
        if (fgets(version, sizeof(version), vf)) {
            size_t len = strlen(version);
            if (len > 0 && version[len-1] == '\n') version[len-1] = 0;
        }
        fclose(vf);
    }
    printf("[版本号] %s\n", version);

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
    
#ifdef QUICKJS_AVAILABLE
    // 测试 QuickJS
    printf("\n=== QuickJS 测试 ===\n");
    
    // 自动遍历 worker 目录下的所有 .js 文件
    DIR *dir;
    char worker_dir[512];
    
    // 获取当前工作目录
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(worker_dir, sizeof(worker_dir), "%s/worker", cwd);
    } else {
        strcpy(worker_dir, "worker");
    }
    
    printf("扫描目录: %s\n", worker_dir);
    dir = opendir(worker_dir);
    
    if (dir != NULL) {
        printf("开始自动执行 JavaScript 文件:\n");
        int executed_count = 0;
        
        // 使用 shell 命令读取目录内容
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "ls -1 %s/*.js 2>/dev/null", worker_dir);
        
        FILE* pipe = popen(cmd, "r");
        if (pipe != NULL) {
            char line[512];
            while (fgets(line, sizeof(line), pipe)) {
                // 移除换行符
                line[strcspn(line, "\n")] = 0;
                
                // 提取文件名
                char* filename = strrchr(line, '/');
                if (filename) {
                    filename++; // 跳过 '/'
                } else {
                    filename = line;
                }
                
                printf("\n--- 自动执行: %s ---\n", filename);
                char* js_content = read_file_content(line);
                if (js_content) {
                    if (execute_javascript(js_content, line) == 0) {
                        printf("✅ %s 执行成功!\n", filename);
                        executed_count++;
                    } else {
                        printf("❌ %s 执行失败!\n", filename);
                    }
                    free(js_content);
                } else {
                    printf("❌ 无法读取文件: %s\n", line);
                }
            }
            pclose(pipe);
        } else {
            printf("无法执行 ls 命令\n");
        }
        
        closedir(dir);
        printf("\n=== 自动执行完成，共执行 %d 个 JavaScript 文件 ===\n", executed_count);
    } else {
        printf("无法打开目录 %s，尝试直接执行已知文件\n", worker_dir);
        
        // 备用方案：直接执行已知文件
        const char* js_files[] = {"worker/test.js", "worker/https_test.js"};
        int num_files = sizeof(js_files) / sizeof(js_files[0]);
        
        for (int i = 0; i < num_files; i++) {
            const char* filepath = js_files[i];
            printf("\n--- 执行文件: %s ---\n", filepath);
            
            char* js_content = read_file_content(filepath);
            if (js_content) {
                if (execute_javascript(js_content, filepath) == 0) {
                    printf("✅ %s 执行成功!\n", filepath);
                } else {
                    printf("❌ %s 执行失败!\n", filepath);
                }
                free(js_content);
            } else {
                printf("❌ 无法读取文件: %s\n", filepath);
            }
        }
    }
#else
    printf("\n=== QuickJS 测试 ===\n");
    printf("QuickJS 功能在此版本中不可用\n");
#endif
    
    printf("\n程序执行完成!\n");
    return 0;
} 