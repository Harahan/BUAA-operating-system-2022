#include<stdio.h>
int main()
{
	int n;
	scanf("%d",&n);

	int tmp = n, ans = 0;
	while(tmp > 0){
		ans = tmp % 10 + ans * 10;
		tmp /= 10;
	}









	if(ans == n){
		printf("Y");
	}else{
		printf("N");
	}
	return 0;
}
