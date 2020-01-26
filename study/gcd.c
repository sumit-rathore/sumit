// Greatest Common Denominator
#include <stdio.h>

int gcd(int a, int b) {
    int remainder = a % b;
    if (remainder == 0) {
        return b;
    }

    return gcd(b, remainder);
}

int Gcd(int num, int *arr)
{
    int result;
    result = (gcd(arr[0], arr[1]));
    for(int i = 0; i < num-1; i++)
    {
        result = (gcd(arr[i], result));
    }
    return result;
}

int main()
{
    int num = 5;
    int input[5] = {3,4,6,8,10};

    printf("%d\n", Gcd(num, &input[0])); 
    //printf("%d\n", gcd(6, 3)); 

    return 0;
}
