// test_post.js - 测试POST数据处理

console.log("=== POST数据测试 ===");

// 获取请求参数
console.log("请求参数:", request_params);

// 解析URL参数
function parseParams(paramsStr) {
    if (!paramsStr || paramsStr.length === 0) {
        return {};
    }
    
    const params = {};
    const pairs = paramsStr.split('&');
    
    for (const pair of pairs) {
        const [key, value] = pair.split('=');
        if (key && value) {
            params[decodeURIComponent(key)] = decodeURIComponent(value);
        }
    }
    
    return params;
}

// 解析参数
const parsedParams = parseParams(request_params);
console.log("解析后的参数:", JSON.stringify(parsedParams, null, 2));

// 处理POST数据
if (parsedParams.action === 'post' || parsedParams.post_data) {
    console.log("=== POST数据处理 ===");
    
    if (parsedParams.name) {
        console.log("用户名:", parsedParams.name);
    }
    
    if (parsedParams.email) {
        console.log("邮箱:", parsedParams.email);
    }
    
    if (parsedParams.message) {
        console.log("消息:", parsedParams.message);
    }
    
    if (parsedParams.post_data) {
        console.log("原始POST数据:", parsedParams.post_data);
        
        // 尝试解析POST数据
        try {
            const postData = JSON.parse(parsedParams.post_data);
            console.log("解析的POST数据:", postData);
        } catch (e) {
            console.log("POST数据不是JSON格式，按字符串处理");
        }
    }
    
    console.log("POST处理完成");
} else {
    console.log("请使用POST方法发送数据");
    console.log("示例: curl -X POST -d 'action=post&name=test&message=hello' http://localhost:8080/test_post.js");
    console.log("或: curl -X POST -d '{\"name\":\"test\",\"message\":\"hello\"}' http://localhost:8080/test_post.js");
}

// 返回结果
const result = {
    status: "success",
    message: "POST数据测试完成",
    received_data: parsedParams,
    timestamp: new Date().toISOString()
};

JSON.stringify(result); 