// HTTPS 功能测试脚本
console.log("=== HTTPS 功能测试 ===");
console.log("执行时间:", new Date().toLocaleString());

// 封装 IP 查询函数
function getMyIP() {
    console.log("正在获取当前IP地址...");
    
    try {
        const response = https_request("api64.ipify.org", "/?format=json", 80);
        if (response) {
            console.log("IP查询成功，响应长度:", response.length);
            console.log("完整响应:", response);
        } else {
            console.log("IP查询失败");
            return null;
        }
    } catch (error) {
        console.log("IP查询异常:", error);
        return null;
    }
}

// 测试 HTTPS 请求
function testHttpsRequest() {
    console.log("\n--- HTTPS 请求测试 ---");
    
    // 测试IP查询
    const myIP = getMyIP();
    if (myIP) {
        console.log("当前IP地址:", myIP);
    }
    
    // 测试其他API
    try {
        const response = https_request("httpbin.org", "/json", 80);
        if (response) {
            console.log("httpbin请求成功，响应长度:", response.length);
            console.log("响应内容前200字符:", response.substring(0, 200));
        } else {
            console.log("httpbin请求失败");
        }
    } catch (error) {
        console.log("请求异常:", error);
    }
}

// 执行测试
testHttpsRequest();

console.log("\n=== HTTPS 测试完成 ==="); 