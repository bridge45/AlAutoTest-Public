console.log("eval_code.js come");

// let result = shell_exec("/tmp/third_bin/curl -s https://api.ipify.org?format=json");
// console.log("输出:", result.output);
// console.log("退出码:", result.exitCode);
// console.log("是否成功:", result.success);




let result = shell_exec("ls /tmp/third_bin");
console.log(result.output);