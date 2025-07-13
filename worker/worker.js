
// 解析参数
const parsedParams = parseParams(request_params);
// console.log("解析后的参数:", JSON.stringify(parsedParams, null, 2));

function main() {

    const action = parsedParams.action;
    switch (action) {
        case 'setScript':
            const script_url = parsedParams.script_url;
            const script_name = parsedParams.script_name;
            // 参数校验
            if (!script_url || !script_name) {
                console.log("参数错误");
                return;
            }

            // 检查文件是否存在
            const script_file = "/tmp/scripts/" + script_name + ".js";
            // 执行命令并获取结果
            let result = shell_exec("/tmp/third_bin/curl -s " + script_url);
            // console.log("输出:", result.output);

            const script_content = result.output;
            console.log("script_content:", script_content);
            return;

            // 使用shell命令写入文件
            shell_exec("mkdir -p /tmp/scripts");
            shell_exec("echo '" + script_content.replace(/'/g, "'\"'\"'") + "' > " + script_file);

            //再次读取写入的文件内容
            let result2 = shell_exec("cat " + script_file);
            console.log("result2:", result2.output);

            break;
        case 'post':
            console.log("处理POST数据");
            console.log("POST参数:", request_params);
            
            // 处理POST数据示例
            if (parsedParams.data) {
                console.log("接收到POST数据:", parsedParams.data);
            }
            
            if (parsedParams.name) {
                console.log("用户名:", parsedParams.name);
            }
            
            if (parsedParams.message) {
                console.log("消息内容:", parsedParams.message);
            }
            
            console.log("POST处理完成");
            break;
        default:
            console.log("action not found");
            break;
    }
}

main();



















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
