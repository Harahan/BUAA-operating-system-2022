all: main.c fibo.c
	gcc -c -I ../include main.c 
	gcc -c -I ../include fibo.c
	gcc -o fibo main.o fibo.o
	mv fibo ../fibo
clean:
	rm fibo.o
	rm main.o
