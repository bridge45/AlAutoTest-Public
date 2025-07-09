
console.log("V4.6"); 


// 执行命令并获取结果
let result = shell_exec("ls -la");
console.log("输出:", result.output);
console.log("退出码:", result.exitCode);
console.log("是否成功:", result.success);

// 执行带错误的命令
let error_result = shell_exec("ls /nonexistent");
console.log("错误输出:", error_result.output);
console.log("错误退出码:", error_result.exitCode);