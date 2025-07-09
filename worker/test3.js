
console.log("V4.6"); 
var start = Date.now();
console.log("start time: " + start);
while (Date.now() - start < 10000) {
    
}

var end = Date.now();
console.log("end time: " + end);
console.log("time cost: " + (end - start)/1000 + "s");