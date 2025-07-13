// dev.js - 演示GET和POST参数使用

console.log("=== 参数演示 ===");

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

// 演示不同参数的使用
if (parsedParams.name) {
    console.log("你好,", parsedParams.name + "!");
}

if (parsedParams.age) {
    console.log("年龄:", parsedParams.age);
}

if (parsedParams.action) {
    console.log("执行操作:", parsedParams.action);
    
    switch (parsedParams.action) {
        case 'test':
            console.log("执行测试操作");
            break;
        case 'info':
            console.log("系统信息:");
            console.log("- 当前时间:", new Date().toISOString());
            console.log("- 参数数量:", Object.keys(parsedParams).length);
            break;
        default:
            console.log("未知操作:", parsedParams.action);
    }
}

// 返回结果
const result = {
    message: "参数处理完成",
    params: parsedParams,
    timestamp: new Date().toISOString()
};

JSON.stringify(result);