console.log("test_simple.js come");

let result = shell_exec("/tmp/third_bin/curl -s https://api.ipify.org?format=json");
console.log("output:", result.output);
console.log("exitCode:", result.exitCode);
console.log("success:", result.success); 