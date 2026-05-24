#include <stdio.h>
#include <stdlib.h>



void rec(int n){
    int x = n;
    printf("n=%d, addr=%p\n",n,(void*)&x);
    if (n <= 0) return;
    
    rec(n-1);
}           


int main(){  

    rec(6);

    return 0;
}                