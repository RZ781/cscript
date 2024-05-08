"expressions";
5;
print;

"printing";
print(5, "\nnewline", print, print("test"));

"ops";
print(-10);

"if";
let t = "true";
let f = "false";
if ("x") {
	let v = t;
	print(v);
} else {
	print(f);
}

if ("") {
	print(t);
} else {
	print(f);
}

if (-16) print(t); else print(f);
if (0) print(t); else print(f);
if (print) print(t); else print(f);

"variables";
let x = 1;
print(x);
x = "hello world";
print(x);

"while";
let x = 1;
let y = 1;
let continue = 1;
while (continue) {
	print(x, y);
	if (y) {
		y = 0;
	} else if (x) {
		x = 0;
		y = 1;
	} else {
		continue = 0;
	}
}
