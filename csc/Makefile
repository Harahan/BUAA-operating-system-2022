all: code/fibo.o code/main.o 
	cd ./code && make
	gcc -o fibo code/main.o code/fibo.o
	cd ./code && mv fibo ../fibo
clean:
	cd ./code && make clean

