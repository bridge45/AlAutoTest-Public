// QuickJS 测试脚本
console.log("=== QuickJS 测试脚本 ===");
console.log("执行时间:", new Date().toLocaleString());

// 数学运算测试
function testMath() {
    console.log("\n--- 数学运算测试 ---");
    let a = 10, b = 5;
    console.log(`${a} + ${b} = ${a + b}`);
    console.log(`${a} * ${b} = ${a * b}`);
    console.log(`${a} ** ${b} = ${a ** b}`);
    console.log(`Math.sqrt(${a}) = ${Math.sqrt(a)}`);
    console.log(`Math.PI = ${Math.PI}`);
}

// 字符串处理测试
function testString() {
    console.log("\n--- 字符串处理测试 ---");
    let message = "Hello QuickJS!";
    console.log("原始字符串:", message);
    console.log("大写:", message.toUpperCase());
    console.log("小写:", message.toLowerCase());
    console.log("长度:", message.length);
    console.log("反转:", message.split('').reverse().join(''));
}

// 数组操作测试
function testArray() {
    console.log("\n--- 数组操作测试 ---");
    let arr = [1, 2, 3, 4, 5];
    console.log("原始数组:", arr);
    console.log("数组和:", arr.reduce((sum, val) => sum + val, 0));
    console.log("数组平方:", arr.map(x => x * x));
    console.log("偶数过滤:", arr.filter(x => x % 2 === 0));
}

// 对象操作测试
function testObject() {
    console.log("\n--- 对象操作测试 ---");
    let obj = {
        name: "QuickJS",
        version: "2021-03-27",
        features: ["ES2020", "Modules", "BigInt"]
    };
    console.log("对象:", JSON.stringify(obj, null, 2));
    console.log("对象键:", Object.keys(obj));
    console.log("对象值:", Object.values(obj));
}

// 执行所有测试
testMath();
testString();
testArray();
testObject();

console.log("\n=== 测试完成V1.4 ==="); 