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
#include <sys/types.h>
#include <execinfo.h>
#include <fcntl.h>
#define _GNU_SOURCE

// 添加必要的函数声明
char* strdup(const char* str);

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

// 自动包含资源头文件
#include "resources/resource_list.h"

// #define PORT 8080  // 注释掉宏定义
#define WORKER_DIR "worker"

// 全局变量
static char* js_result = NULL;
static struct MHD_Daemon *g_daemon = NULL;
static char console_output[8192] = ""; // 存储console.log输出
static char worker_dir[256] = WORKER_DIR; // 新增全局 worker_dir

// 新增端口检测和放行函数
int is_port_allowed(int port) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "iptables -C INPUT -p tcp --dport %d -j ACCEPT > /dev/null 2>&1", port);
    int ret = system(cmd);
    return ret == 0;
}
void allow_port(int port) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "iptables -A INPUT -p tcp --dport %d -j ACCEPT", port);
    system(cmd);
}

// 信号处理函数
static void signal_handler(int sig) {
#ifdef MICROHTTPD_AVAILABLE
    if (g_daemon != NULL) {
        MHD_stop_daemon(g_daemon);
        g_daemon = NULL;
    }
#endif
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
            // 检查缓冲区空间并安全追加到console_output
            size_t current_len = strlen(console_output);
            size_t remaining_space = sizeof(console_output) - current_len - 1;
            if (remaining_space > 0) {
                strncat(console_output, str, remaining_space);
            }
            JS_FreeCString(ctx, str);
        }
        if (i < argc - 1) {
            printf(" ");
            size_t current_len = strlen(console_output);
            size_t remaining_space = sizeof(console_output) - current_len - 1;
            if (remaining_space > 0) {
                strncat(console_output, " ", remaining_space);
            }
        }
    }
    printf("\n");
    size_t current_len = strlen(console_output);
    size_t remaining_space = sizeof(console_output) - current_len - 1;
    if (remaining_space > 0) {
        strncat(console_output, "\n", remaining_space);
    }
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

// shell_exec 实现
static JSValue js_shell_exec(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    (void)this_val;
    if (argc < 1) {
        return JS_EXCEPTION;
    }
    
    const char* command = JS_ToCString(ctx, argv[0]);
    if (!command) {
        return JS_EXCEPTION;
    }
    
    // 创建管道
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        JS_FreeCString(ctx, command);
        return JS_NULL;
    }
    
    // 创建子进程
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        JS_FreeCString(ctx, command);
        return JS_NULL;
    }
    
    if (pid == 0) {
        // 子进程
        close(pipefd[0]); // 关闭读端
        dup2(pipefd[1], STDOUT_FILENO); // 重定向标准输出
        dup2(pipefd[1], STDERR_FILENO); // 重定向标准错误
        close(pipefd[1]);
        
        // 执行命令
        execl("/bin/sh", "sh", "-c", command, NULL);
        exit(1);
    } else {
        // 父进程
        close(pipefd[1]); // 关闭写端
        
        // 读取输出
        char buffer[4096];
        char* output = malloc(8192);
        output[0] = '\0';
        int total_size = 0;
        
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            size_t current_len = strlen(output);
            size_t remaining_space = 8192 - current_len - 1;
            if (remaining_space > 0) {
                strncat(output, buffer, remaining_space);
            }
            total_size += bytes_read;
            
            if (total_size > 8000) break;
        }
        
        close(pipefd[0]);
        
        // 等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        
        // 检查退出状态
        int exit_code = WEXITSTATUS(status);
        
        // 创建返回对象
        JSValue result_obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, result_obj, "output", JS_NewString(ctx, output));
        JS_SetPropertyStr(ctx, result_obj, "exitCode", JS_NewInt32(ctx, exit_code));
        JS_SetPropertyStr(ctx, result_obj, "success", JS_NewBool(ctx, exit_code == 0));
        
        free(output);
        JS_FreeCString(ctx, command);
        return result_obj;
    }
}

#ifdef BEARSSL_AVAILABLE
// HTTPS 请求函数实现
char* https_request(const char* host, const char* path, int port) {
    return NULL;
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
    
    // 添加 shell_exec 函数到全局对象
    JS_SetPropertyStr(ctx, global_obj, "shell_exec", 
        JS_NewCFunction(ctx, js_shell_exec, "shell_exec", 1));
    
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
        snprintf(filepath, sizeof(filepath), "%s/%s", worker_dir, js_file);
        
        if (file_exists(filepath)) {
            char* js_content = read_file_content(filepath);
            if (js_content) {
                char* result = execute_javascript(js_content, filepath);
                free(js_content);
                
                // 解析JSON并提取return和console
                char* return_start = strstr(result, "\"return\":\"");
                char* console_start = strstr(result, "\"console\":\"");
                
                if (return_start && console_start) {
                    return_start += 10; // 跳过 "return":"
                    console_start += 11; // 跳过 "console":"
                    
                    char* return_end = strchr(return_start, '"');
                    char* console_end = strrchr(console_start, '"');
                    
                    if (return_end && console_end) {
                        *return_end = '\0';
                        *console_end = '\0';
                        
                        // 如果return是undefined，只输出console
                        if (strcmp(return_start, "undefined") == 0) {
                            response_data = strdup(console_start);
                        } else {
                            // 构建最终输出: return + 换行符 + console
                            int total_len = strlen(return_start) + 1 + strlen(console_start) + 1;
                            char* final_output = malloc(total_len);
                            snprintf(final_output, total_len, "%s\n%s", return_start, console_start);
                            response_data = final_output;
                        }
                    } else {
                        response_data = result;
                    }
                } else {
                    response_data = result;
                }
            } else {
                response_data = strdup("{\"status\":\"error\",\"message\":\"无法读取JS文件\"}");
                status_code = 500;
            }
        } else {
            char notfound_msg[600];
            snprintf(notfound_msg, sizeof(notfound_msg), "{\"status\":\"error\",\"message\":\"JS文件不存在: %s\"}", filepath);
            response_data = strdup(notfound_msg);
            status_code = 404;
        }
    }
    
    response = MHD_create_response_from_buffer(strlen(response_data),
                                              response_data,
                                              MHD_RESPMEM_MUST_FREE);
    
    
    
                                              MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    
    return (enum MHD_Result)ret;
}
#endif

void segfault_handler(int sig) {
    void *array[20];
    size_t size = backtrace(array, 20);
    fprintf(stderr, "捕获到信号 %d (Segmentation fault)，调用栈如下：\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

// 主函数
int main(int argc, char **argv) {
    // 资源释放到 /tmp/third_bin/
    const char *out_dir = "/tmp/third_bin";
    struct stat st = {0};
    if (stat(out_dir, &st) == -1) {
        mkdir(out_dir, 0700);
    }
    for (size_t i = 0; i < get_resources_count(); ++i) {
        char out_path[512];
        Resource* resources = get_resources();
        snprintf(out_path, sizeof(out_path), "%s/%s", out_dir, resources[i].name);
        if (stat(out_path, &st) == -1) {
            int fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
            if (fd >= 0) {
                write(fd, resources[i].data, resources[i].len);
                close(fd);
                chmod(out_path, 0755);
            }
        }
    }
    signal(SIGSEGV, segfault_handler);
    printf("输入参数: ");
    for (int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    int port = 8080;
    // 解析参数
    for (int i = 1; i < argc - 1; ++i) {
        if (strcmp(argv[i], "--port") == 0) {
            port = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "--wdir") == 0) {
            strncpy(worker_dir, argv[i + 1], sizeof(worker_dir) - 1);
            worker_dir[sizeof(worker_dir) - 1] = '\0';
        }
    }
    printf("=== ARMv7 Web Server with QuickJS ===\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("目标架构: ARMv7\n");
    printf("Web服务端口: %d\n", port);
    printf("工作目录: %s\n\n", worker_dir);
    
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
    
    // 检查端口防火墙规则
    if (!is_port_allowed(port)) {
        printf("检测到端口未放行，自动执行放行...\n");
        allow_port(port);
    }
    
    // 创建HTTP服务器
    g_daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL,
                               &request_handler, NULL, MHD_OPTION_CONNECTION_TIMEOUT, 5, MHD_OPTION_END);
    
    if (g_daemon == NULL) {
        fprintf(stderr, "无法启动HTTP服务器\n");
        return 1;
    }
    
    printf("HTTP服务器已启动，监听端口 %d\n", port);
    printf("访问 http://localhost:%d 查看服务\n", port);
    printf("访问 http://localhost:%d/js/文件名.js 执行JS文件\n", port);
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
    snprintf(worker_path, sizeof(worker_path), "%s", worker_dir);
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