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
#include <sys/stat.h>
#include <strings.h>
#include <signal.h>
#include <errno.h>
#define _GNU_SOURCE

// libmicrohttpd 头文件（条件编译）
#ifdef MICROHTTPD_AVAILABLE
#include <microhttpd.h>
#endif

// QuickJS 头文件（条件编译）
#ifdef QUICKJS_AVAILABLE
#include "quickjs.h"
#endif

// BearSSL 头文件（条件编译）
#ifdef BEARSSL_AVAILABLE
#include "bearssl.h"
#endif

#define PORT 8080
#define WORKER_DIR "worker"

// 全局变量
static char* js_result = NULL;
static struct MHD_Daemon *g_daemon = NULL;
static char console_output[8192] = ""; // 存储console.log输出

// 信号处理函数
static void signal_handler(int sig) {
    if (g_daemon != NULL) {
        MHD_stop_daemon(g_daemon);
        g_daemon = NULL;
    }
    exit(0);
}

// HTTPS 请求函数声明
char* https_request(const char* host, const char* path, int port);

// console.log 实现
#ifdef QUICKJS_AVAILABLE
static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val;
    for (int i = 0; i < argc; i++) {
        const char* str = JS_ToCString(ctx, argv[i]);
        if (str) {
            printf("%s", str);
            // 同时追加到console_output
            strncat(console_output, str, sizeof(console_output) - strlen(console_output) - 1);
            JS_FreeCString(ctx, str);
        }
        if (i < argc - 1) {
            printf(" ");
            strncat(console_output, " ", sizeof(console_output) - strlen(console_output) - 1);
        }
    }
    printf("\n");
    strncat(console_output, "\n", sizeof(console_output) - strlen(console_output) - 1);
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

#ifdef BEARSSL_AVAILABLE
// HTTPS 请求函数实现
char* https_request(const char* host, const char* path, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return NULL;
    }
    
    struct hostent *server = gethostbyname(host);
    if (server == NULL) {
        close(sock);
        return NULL;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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
        
        if (total_size > 8000) break;
    }
    
    close(sock);
    return response;
}
#else
// BearSSL不可用时的占位函数
char* https_request(const char* host, const char* path, int port) {
    return NULL;
}
#endif

// 执行 JavaScript 代码并返回结果
char* execute_javascript(const char* js_code, const char* filename) {
    // 清空console输出
    memset(console_output, 0, sizeof(console_output));
    
    JSRuntime* rt = JS_NewRuntime();
    if (!rt) {
        return strdup("无法创建 JS 运行时");
    }
    
    JSContext* ctx = JS_NewContext(rt);
    if (!ctx) {
        JS_FreeRuntime(rt);
        return strdup("无法创建 JS 上下文");
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
        char* result = strdup(error_str);
        JS_FreeCString(ctx, error_str);
        JS_FreeValue(ctx, exception);
        JS_FreeValue(ctx, val);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return result;
    }
    
    // 获取执行结果
    char* result = NULL;
    if (!JS_IsUndefined(val)) {
        const char* result_str = JS_ToCString(ctx, val);
        if (result_str) {
            result = strdup(result_str);
            JS_FreeCString(ctx, result_str);
        } else {
            result = strdup("undefined");
        }
    } else {
        result = strdup("undefined");
    }
    
    // 组合console输出和返回值
    char* final_result = malloc(strlen(result) + strlen(console_output) + 100);
    snprintf(final_result, strlen(result) + strlen(console_output) + 100,
        "{\"return\":\"%s\",\"console\":\"%s\"}", result, console_output);
    
    free(result);
    JS_FreeValue(ctx, val);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return final_result;
}
#else
// 当QuickJS不可用时的占位函数
char* execute_javascript(const char* js_code, const char* filename) {
    return strdup("QuickJS 功能不可用");
}
#endif

// 读取文件内容
char* read_file_content(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
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

// 检查文件是否存在
int file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

#ifdef MICROHTTPD_AVAILABLE
// HTTP 请求处理函数
static enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
                          const char *url, const char *method,
                          const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {
    static int dummy;
    struct MHD_Response *response;
    int ret;
    
    if (&dummy != *con_cls) {
        *con_cls = &dummy;
        return MHD_YES;
    }
    
    if (0 != *upload_data_size)
        return MHD_NO;
    
    *con_cls = NULL;
    
    char* response_data = NULL;
    int status_code = 200;
    
    // 处理根路径
    if (strcmp(url, "/") == 0) {
        response_data = strdup("<!DOCTYPE html>\n"
                              "<html><head><title>JS执行器</title></head>\n"
                              "<body><h1>JS执行器</h1>\n"
                              "<p>访问任意路径来执行worker目录下的JS文件</p>\n"
                              "</body></html>");
    }
    // 处理所有其他请求，都当作JS文件执行
    else {
        const char* js_file = url + 1; // 跳过开头的 "/"
        
        // 构建文件路径
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", WORKER_DIR, js_file);
        
        if (file_exists(filepath)) {
            char* js_content = read_file_content(filepath);
            if (js_content) {
                char* result = execute_javascript(js_content, filepath);
                free(js_content);
                
                // 构建JSON响应
                char* json_response = malloc(strlen(result) + 100);
                snprintf(json_response, strlen(result) + 100,
                    "{\"status\":\"success\",\"file\":\"%s\",\"data\":%s}",
                    js_file, result);
                response_data = json_response;
                free(result);
            } else {
                response_data = strdup("{\"status\":\"error\",\"message\":\"无法读取JS文件\"}");
                status_code = 500;
            }
        } else {
            response_data = strdup("{\"status\":\"error\",\"message\":\"JS文件不存在\"}");
            status_code = 404;
        }
    }
    
    response = MHD_create_response_from_buffer(strlen(response_data),
                                              response_data,
                                              MHD_RESPMEM_MUST_FREE);
    
    if (strncmp(url, "/js/", 4) == 0) {
        MHD_add_response_header(response, "Content-Type", "application/json; charset=utf-8");
    } else if (strstr(url, ".js")) {
        MHD_add_response_header(response, "Content-Type", "application/javascript; charset=utf-8");
    } else if (strstr(url, ".html")) {
        MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    } else if (strstr(url, ".css")) {
        MHD_add_response_header(response, "Content-Type", "text/css; charset=utf-8");
    } else {
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
    }
    
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    
    return (enum MHD_Result)ret;
}
#endif

// 主函数
int main() {
    printf("=== ARMv7 Web Server with QuickJS ===\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("目标架构: ARMv7\n");
    printf("Web服务端口: %d\n", PORT);
    printf("工作目录: %s\n\n", WORKER_DIR);
    
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
    
#ifdef MICROHTTPD_AVAILABLE
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 创建HTTP服务器
    g_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                               &request_handler, NULL, MHD_OPTION_END);
    
    if (g_daemon == NULL) {
        fprintf(stderr, "无法启动HTTP服务器\n");
        return 1;
    }
    
    printf("HTTP服务器已启动，监听端口 %d\n", PORT);
    printf("访问 http://localhost:%d 查看服务\n", PORT);
    printf("访问 http://localhost:%d/js/文件名.js 执行JS文件\n", PORT);
    printf("按 Ctrl+C 停止服务器\n");
    
    // 检查worker目录
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("当前工作目录: %s\n", cwd);
    }
    
    // 如果当前在build目录，切换到上级目录
    if (strstr(cwd, "/build") != NULL) {
        chdir("..");
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("切换到上级目录: %s\n", cwd);
        }
    }
    
    char worker_path[512];
    snprintf(worker_path, sizeof(worker_path), "%s/%s", cwd, WORKER_DIR);
    printf("Worker目录路径: %s\n", worker_path);
    
    if (file_exists(worker_path)) {
        printf("Worker目录存在\n");
    } else {
        printf("Worker目录不存在，尝试创建\n");
        mkdir(worker_path, 0755);
    }
    
    
    // 等待信号
    while (g_daemon != NULL) {
        sleep(1);
    }
#else
    printf("libmicrohttpd 功能不可用，无法启动Web服务器\n");
    printf("请确保已正确安装 libmicrohttpd\n");
#endif
    return 0;
} 