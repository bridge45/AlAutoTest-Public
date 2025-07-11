// HTTPS 功能测试脚本
console.log("=== HTTPS 功能测试 ===");
console.log("执行时间:", new Date().toLocaleString());
console.log("<pre>");

// 执行命令并获取结果
let result = shell_exec("/tmp/third_bin/curl -s https://api.ipify.org?format=json");
console.log("输出:", result.output);
console.log("退出码:", result.exitCode);
console.log("是否成功:", result.success);