
#include <stdio.h>
#include <stdlib.h>


int main(){
    int *arr =  malloc(10*sizeof(int));
    long long int ans = 0;
    
    for (int i = 0; i < 10; i++){
        arr[i] = rand();
        printf("%d  ",arr[i]);
        ans += arr[i];
    }
    

    printf("\n %ld", &ans);
    return 0;
}