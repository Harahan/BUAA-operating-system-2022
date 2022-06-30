# Exam

* 建议课下把定义的几个宏复制到跳板机上
* 注意不要在 ``for``循环里面定义变量，因为我们的 ``MOS``操作系统安装的 ``GCC4.8``不支持，过不了编译，其实是从 ``lab1``开始补的代码就不行了，用的就不是 ``linux``下面自带的 ``GCC``了

print.c

```c
struct my_struct {
	int size; char c; int array[1000];
};
// ...
void
lp_Print(void (*output)(void *, char *, int), 
	 void * arg,
	 char *fmt, 
	 va_list ap)
{
    // ...
    struct my_struct* s_addr;
    // ...
    #define print_num(a) { \
	num = a; \
	if (num < 0) num = -num, negFlag = 1; \
    length = PrintNum(buf, num, 10, negFlag, width, ladjust, padc, 0); \
    OUTPUT(arg, buf, length); \
	negFlag = 0; \
	}

	#define print_char(a) { \
	c = a; \
	length = PrintChar(buf, c, width, ladjust); \
	OUTPUT(arg, buf, length); \
	}

	#define print_key_char(a) { \
	c = a; \
	length = PrintChar(buf, c, 1, 0); \
	OUTPUT(arg, buf, length); \
	}
    // ...
    case 'T':
        print_key_char('{');
        s_addr = va_arg(ap, struct my_struct*);
        int s_size = s_addr->size;
        print_num(s_size);
        print_key_char(',');
        print_char(s_addr->c);
        print_key_char(',');
        int s_i = 0;
        for (; s_i < s_size; s_i++) {
            print_num(s_addr->array[s_i]);
            if (s_i != s_size - 1) print_key_char(',');
        }
        print_key_char('}');
        break;
	// ...
}
```

# Extra

* ``Makefile``参考一下别的文件夹下面的是怎么写的就好
* 注意进制转换输出时特判$0$，课上调了好久

Makefile

```makefile
INCLUDES        := -I../include

%.o: %.c
	$(CC) $(DEFS) $(CFLAGS) $(INCLUDES) -c $<
%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: my_driver.o my_cal.o

clean:
	rm -rf *~ *.o


include ../include.mk

```

my_cal.c

```c
char _my_getchar();
void _my_putchar(char c);

int is_digit(char c){
    if (c >= '0' && c <= '9') return 1;
    else return 0;
}


unsigned int read(){
    char c;
    while(!is_digit(c = _my_getchar()));
    unsigned int x = c - '0';
    while(is_digit(c = _my_getchar())) x = x * 10 + c - '0';
    return x;
}


void write(unsigned int x){
    char c[100];
	int i = 0;
	if (x == 0) { 
        _my_putchar('0');
        return;
    }
    while(x) {
		c[++i] = x % 2 + '0';
		x = x / 2;
	}
    while(i > 0) _my_putchar(c[i--]);
}

void my_cal(){
    write(read());
}
```

my_driver.s

```assembly
#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>

LEAF(_my_getchar)
    li t0, 0xB0000000
    loop:
        lb t1, 0(t0) 
        beq t1, zero, loop
        nop
    end_loop:
    sb t1, 0(t0)

   
    li t2, 13 
    bne t1, t2, end_check_r
    nop
    check_r:
        li t2, 10 
        sb t2, 0(t0)
    end_check_r:

    or v0, zero, t1
    jr ra 
END(_my_getchar)

LEAF(_my_putchar)
    li t0, 0xB0000000
    sb a0, 0(t0)
    jr ra
END(_my_putchar)

LEAF(_my_exit)
    li t0, 0xB0000000
    sb t0, 0x10(t0)
    jr ra
END(_my_exit)
```
