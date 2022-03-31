char _my_getchar();
void _my_putchar(char c);
//#include<stdio.h>

int isDigit(char c){
    if (c >= '0' && c <= '9') return 1;
    else return 0;
}


unsigned int read(){
    char c;
    while(!isDigit(c = _my_getchar()));
    unsigned int x = c - '0';
    while(isDigit(c = _my_getchar())) x = x * 10 + c - '0';
    return x;
}


void write(unsigned int x){
    char c[100];
	int i = 0;
	if (x == 0){ _my_putchar('0');return;}
    while(x) {
		c[++i] = x % 2 + '0';
		x = x / 2;
	}
    while(i > 0) _my_putchar(c[i--]);
}

void my_cal(){
    write(read());
}



