# Exam

Makefile

```makefile
all: hello_os.c
	gcc hello_os.c -o os_hello
clean:
	rm os_hello
```

lab0-exam.sh

```sh
#!/bin/bash
make
touch hello_os
mkdir hello_os_dir
cp os_hello ./hello_os_dir
cp os_hello ./hello_os_dir/hello_os
rm os_hello
grep -n -i os_hello ./hello_os.c > hello_os.txt
```

# Extra

建议大家不要用 ``each `xxx` ``,它会把多行输出输到一行，相当于吞掉换行符替换为空格，比如 ``each `./test` ``：

```c
int main() {
	for(int i = 0; i < 3; i++) {
		printf("%d\n", i);
	}
}
```

输出则为：

```c
0 1 2
```

find_warnings.sh

```sh
#!/bin/bash
gcc -Wall $1 -o test 2> warning.txt
grep warning ./warning.txt > ./result.txt
sed  -i 's/warning: //g' ./result.txt
gcc $1 -o test
if [ $? -eq 0 ]
then
		a=1
		while [ $a -le $2 ]
		do
				echo $a | ./test >> result.txt
				a=$[$a+1]
		done
fi
pwd >> result.txt
```
