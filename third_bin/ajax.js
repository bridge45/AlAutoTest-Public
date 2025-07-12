
console.log("V1.2"); 


// 执行命令并获取结果
let result = shell_exec("ls -la /tmp/third_bin/curl && /tmp/third_bin/curl -k --ssl-no-revoke https://www.google.com");
console.log("输出:", result.output);
console.log("退出码:", result.exitCode);
console.log("是否成功:", result.success);
